#pragma once

#include <utils/utils.h>
#include <string>
#include <vector>
#include <memory>

class Bulkmlt;

class IInterpreterState
{
    public:
        IInterpreterState(Bulkmlt& bulkmlt)
                : _bulkmlt(bulkmlt)
        {
        }

        virtual ~IInterpreterState()
        {
        }

        virtual void Initialize() {};
        virtual void Exec(std::string ctx) = 0;
        virtual void Finalize() {};

    protected:
        Bulkmlt& _bulkmlt;
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
};
