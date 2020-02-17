#pragma once

#include <utils/utils.h>
#include <string>
#include <vector>
#include <memory>

class Bulk;

class IInterpreterState
{
    public:
        IInterpreterState(Bulk& bulk)
                : _bulk(bulk)
        {
        }

        virtual ~IInterpreterState()
        {
        }

        virtual void Initialize() {};
        virtual void Exec(std::string ctx) = 0;
        virtual void Finalize() {};

    protected:
        Bulk& _bulk;
};


struct IExpression
{
    virtual ~IExpression()
    {
    }

    virtual operator std::string() const = 0;
};

struct Command: public IExpression
{
    std::string value;

    virtual operator std::string() const override
    {
        return value;
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
};
