/* Written by Todd Doucet.
 */
#pragma once

#include <string>
#include <vector>
#include <queue>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>

namespace detail_cmdline
{
    using namespace std;

    class cmdline
    {
    public:
        vector<string> Args;
        vector<string> ExtraArgs;

        void parse(int argc, char *argv[])
        {
            for (; argc; --argc, ++argv)
                Args.push_back(argv[0]);

            try
            {
                for (const string& s : Args)
                {
                    if (s[0] == '-')
                    {
                        if  (s[1] == '-')
                            do_flag(s);
                        else
                            for (char ch : s.substr(1))
                                do_flag(ch);
                    }
                    else if (need.empty())
                        ExtraArgs.push_back(s);
                    else
                    {
                        int i = need.front();
                        need.pop();
                        flags[i]->set_value(s);
                    }
                }

                if (need.empty() == false)
                    throw runtime_error("Missing parameter value"s);
            }
            catch(exception& e)
            {
                usage(e.what());
            }
        }

        template<typename T>
        void setopt(char flag, T& val, string help)
        {
            auto e = make_unique<entry<T>>(flag, ""s, val, help);
            flags.push_back(move(e));
        }

        template<typename T>
        void setopt(string longform, T& val, string help)
        {
            auto e = make_unique<entry<T>>(' ', longform, val, help);
            flags.push_back(move(e));
        }

        template<typename T>
        void setopt(char flag, string longform, T& val, string help)
        {
            auto e = make_unique<entry<T>>(flag, longform, val, help);
            flags.push_back(move(e));
        }

        void usage(const string& s=""s)
        {
            if (s.length())
                cerr << s << "\n";

            cerr << "Usage: " << Args[0] << " [options] [args]" << "\n";
            cerr << summary << "\n";
            cerr << "Options:\n" << std::left;

            for (auto& flag : flags)
            {
                bool has_short = (flag->flag != ' ');
                if (has_short)
                    cerr << "  -" << flag->flag;
                else
                    cerr << "    ";

                string col2;
                if (flag->longform.length())
                    col2 = (has_short? ", "s : "  "s) + flag->longform;

                cerr << setw(15) << col2;
                cerr << " " << flag->help << "\n";
            }
            cerr << "    " << setw(15) << "  --help" << " Print this message and exit.\n";
            std::exit(1);
        }

        void set_summary(const string& text)
        {
            summary = text;
        }

    private:
        int find_opt(char ch)
        {
            for (size_t i = 0; i < flags.size(); ++i)
                if (flags[i]->flag == ch)
                    return i;
            throw runtime_error("Unknown switch: -"s + ch);
        }

        int find_opt(const string& longform)
        {
            for (size_t i = 0; i < flags.size(); ++i)
                if (flags[i]->longform == longform)
                    return i;
            if (longform == "--help")
                usage();
            throw runtime_error("Unknown switch: "s + longform);
        }

        template<typename T>
        void do_flag(T flag)
        {
            int i = find_opt(flag);
            if (flags[i]->needs_arg)
                need.push(i);
            else
                flags[i]->set_value(""s);
        }

        struct ebase
        {
            char flag;
            string help;
            string longform;
            bool needs_arg;

            ebase(char f, string lform, string h, bool needs = true)
                : flag(f), help(h), longform(lform), needs_arg(needs) { }

            // We do not need to know the type of the destination here.
            // We only need to be able to set it from a string.
            virtual void set_value(const string& s) = 0;
            virtual ~ebase() = default;
        };

        template<typename T>
        struct entry: public ebase
        {
            T& val;

            entry(char flag, string longform, T& v, string help)
                : ebase{flag, longform, help}, val{v}  { }

            void set_value(const string& s) override
            {
                stringstream ss{s};
                ss >> val;
                if (ss.fail() || !ss.eof())
                    throw runtime_error("unexpected value: '"s + s + "'"s);
            }
        };

        vector<unique_ptr<ebase>> flags;
        queue<int> need;
        string summary;
    };

    template<>
    struct cmdline::entry<bool> : public cmdline::ebase
    {
        bool& val;

        entry(char flag, string lform, bool& v, string help)
            : ebase{flag, lform, help, false}, val{v} { }

        void set_value(const string&) override { val = !val; }
    };
} // namespace

using detail_cmdline::cmdline;
