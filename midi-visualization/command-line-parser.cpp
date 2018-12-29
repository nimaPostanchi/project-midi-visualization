#include "command-line-parser.h"
#include <iostream>
#include <assert.h>


void CommandLineParser::register_processor(const std::string& prefix, std::function<void()> processor)
{
    std::function<void(std::list<std::string>&)> wrapper = [processor](std::list<std::string>& arguments) -> void {
        processor();
    };

    register_processor(prefix, wrapper);
}

void CommandLineParser::register_processor(const std::string& prefix, std::function<void(const std::string&)> processor)
{
    std::function<void(std::list<std::string>&)> wrapper = [prefix, processor](std::list<std::string>& arguments) -> void {
        if (arguments.empty())
        {
            std::cerr << "Command line argument " << prefix << " expects an argument";
            abort();
        }
        auto head = arguments.front();
        arguments.pop_front();

        processor(head);
    };

    register_processor(prefix, wrapper);
}

void CommandLineParser::register_processor(const std::string& prefix, std::function<void(int)> processor)
{
    std::function<void(const std::string&)> wrapper = [prefix, processor](const std::string& argument) -> void {
        try
        {
            auto value = std::stoi(argument);
            processor(value);
        }
        catch (const std::invalid_argument&)
        {
            std::cerr << "Invalid value for " << prefix << std::endl;
            abort();
        }
    };

    register_processor(prefix, wrapper);
}

void CommandLineParser::register_processor(const std::string& prefix, std::function<void(std::list<std::string>&)> processor)
{
    if (is_prefix_in_use(prefix))
    {
        std::cerr << "Clashing prefixes";
        abort();
    }
    else
    {
        m_map[prefix] = processor;
    }
}

void CommandLineParser::process(int argc, char** argv) const
{
    std::list<std::string> arguments;

    for (int i = 1; i < argc; ++i)
    {
        arguments.push_back(argv[i]);
    }

    process(arguments);
}

void CommandLineParser::process(std::list<std::string>& arguments) const
{
    while (!arguments.empty())
    {
        auto head = arguments.front();
        arguments.pop_front();

        auto it = m_map.find(head);

        if (it == m_map.end())
        {
            std::cerr << "Unknown command " << head;
            abort();
        }
        else
        {
            auto processor = *it;
            processor.second(arguments);
        }
    }
}

bool CommandLineParser::is_prefix_in_use(const std::string& prefix) const
{
    return m_map.find(prefix) != m_map.end();
}
