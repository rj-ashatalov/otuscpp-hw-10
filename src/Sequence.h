#pragma once
#include <string>
#include <vector>
#include "IInterpreterState.h"

class Sequence: public IInterpreterState
{
    public:
        Sequence(Bulkmlt& bulkmlt);
        virtual void Exec(std::string ctx) override;
        virtual void Initialize() override;
        virtual void Finalize() override;

    private:
        Group _commands;
};
