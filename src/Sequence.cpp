#include "Sequence.h"
#include "Bulk.h"
#include <iostream>
#include <ctime>

Sequence::Sequence(Bulk& bulk)
        : IInterpreterState(bulk)
{

}

void Sequence::Exec(std::string ctx)
{
    if (ctx == "{")
    {
        _bulk.SetState<InfinitSequence>();
        return;
    }

    auto command = std::make_shared<Command>();
    command->value = ctx;
    _commands.expressions.push_back(command);

    if (_commands.expressions.size() == 1)
    {
        _bulk.eventFirstCommand.Dispatch(std::time(nullptr));
    }

    if (_commands.expressions.size() >= static_cast<size_t>(_bulk.commandBufCount))
    {
        _bulk.SetState<Sequence>();
    }
}

void Sequence::Initialize()
{
    IInterpreterState::Initialize();
    _commands.expressions.clear();
}

void Sequence::Finalize()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    IInterpreterState::Finalize();
    if (_commands.expressions.size() > 0)
    {
        _bulk.eventSequenceComplete.Dispatch(_commands);
    }
}




