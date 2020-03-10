#pragma once

#include <utils/utils.h>
#include <string>
#include <vector>
#include <memory>

class Bulkmt;

class IInterpreterState
{
    public:
        IInterpreterState(Bulkmt& bulkmt)
                : _bulkmt(bulkmt)
        {
        }

        virtual ~IInterpreterState()
        {
        }

        virtual void Initialize() {};
        virtual void Exec(std::string ctx) = 0;
        virtual void Finalize() {};

    protected:
        Bulkmt& _bulkmt;
};


struct IExpression
{
    virtual ~IExpression()
    {
    }

    virtual operator std::string() const = 0;
    virtual int Size() const = 0;
};

struct Command: public IExpression
{
    std::string value;

    virtual operator std::string() const override
    {
        return value;
    }

    virtual int Size() const override
    {
        return 1;
    }
};

struct Group: public IExpression
{
    std::shared_ptr<Group> parent;
    std::vector<std::shared_ptr<IExpression>> expressions;

    virtual operator std::string() const override
    {
        return Utils::Join(expressions, ", ");
    }

    virtual int Size() const override
    {
        int result = 0;
        for (auto&& expression : expressions)
        {
            result += expression->Size();
        }
        return result;
    }

    std::vector<std::shared_ptr<Command>> Merge()
    {
        std::vector<std::shared_ptr<Command>> result{};
        if (expressions.size() > 0)
        {
            Concat(result, expressions);
        }
        return result;
    }

    private:
        void Concat(std::vector<std::shared_ptr<Command>>& dest, std::vector<std::shared_ptr<IExpression>>& src)
        {
            if (src.size() <= 0)
            {
                return;
            }

            std::for_each(src.begin(), src.end(), [&dest, this](auto& item)
            {
                auto command = std::dynamic_pointer_cast<Command>(item);
                if (command)
                {
                    dest.push_back(command);
                }

                auto group = std::dynamic_pointer_cast<Group>(item);
                if (group)
                {
                    Concat(dest, group->expressions);
                }
            });
        }
};
