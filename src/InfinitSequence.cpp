#include "InfinitSequence.h"
#include "Bulkmt.h"
#include <iostream>
#include <ctime>

InfinitSequence::InfinitSequence(Bulkmt& bulkmt)
        : IInterpreterState(bulkmt)
{

}

void InfinitSequence::Initialize()
{
    IInterpreterState::Initialize();
    if (_rootGroup == nullptr)
    {
        _currentGroup = std::make_shared<Group>();
        _rootGroup = _currentGroup;
        _awaitFirstCommand = true;
    }
}

void InfinitSequence::Finalize()
{
    IInterpreterState::Finalize();
    if (_currentGroup == nullptr)
    {
        {
            std::unique_lock<std::mutex> locker(Utils::lockPrint);
            std::cout << __PRETTY_FUNCTION__ << " Expression complete" << std::endl;
        }
        if (!_awaitFirstCommand)
        {
            _bulkmt.eventSequenceComplete.Dispatch(_rootGroup);
        }
        _rootGroup = nullptr;
    }
}

void InfinitSequence::Exec(std::string ctx)
{
    if (ctx == "{")
    {
        auto group = std::make_shared<Group>();
        group->parent = _currentGroup;
        _currentGroup = group;
        _bulkmt.SetState<InfinitSequence>();
        return;
    }

    if (ctx == "}")
    {
        if (_currentGroup->parent == nullptr)
        {
            _currentGroup = nullptr;
            _bulkmt.SetState<Sequence>();
        }
        else
        {
            _currentGroup->parent->expressions.push_back(_currentGroup);
            _currentGroup = _currentGroup->parent;
            _bulkmt.SetState<InfinitSequence>();
        }
        return;
    }

    auto command = std::make_shared<Command>();
    command->value = ctx;
    _currentGroup->expressions.push_back(command);
    if (_awaitFirstCommand)
    {
        _awaitFirstCommand = false;
        _bulkmt.eventFirstCommand.Dispatch(std::time(nullptr));
    }
}

