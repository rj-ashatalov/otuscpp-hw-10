#pragma once

#include "IInterpreterState.h"
#include <string>

class InfinitSequence: public IInterpreterState
{
    public:
        InfinitSequence(Bulk& bulk);
        virtual void Initialize() override;
        virtual void Finalize() override;
        virtual void Exec(std::string ctx) override;

    protected:
        std::shared_ptr<Group> _rootGroup;
        std::shared_ptr<Group> _currentGroup;
        bool _awaitFirstCommand = true;
};


