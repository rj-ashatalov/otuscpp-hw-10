#include "Sequence.h"
#include "Bulkmt.h"
#include <iostream>
#include <ctime>

Sequence::Sequence(Bulkmt& bulkmt)
        : IInterpreterState(bulkmt)
{

}

void Sequence::Exec(std::string ctx)
{
    if (ctx == "{")
    {
        _bulkmt.SetState<InfinitSequence>();
        return;
    }

    auto command = std::make_shared<Command>();
    command->value = ctx;
    _commands->expressions.push_back(command);

    if (_commands->expressions.size() == 1)
    {
        _bulkmt.eventFirstCommand.Dispatch(std::time(nullptr));
    }

    if (_commands->expressions.size() >= static_cast<size_t>(_bulkmt.commandBufCount))
    {
        _bulkmt.SetState<Sequence>();
    }
}

void Sequence::Initialize()
{
    IInterpreterState::Initialize();
    _commands.reset(new Group());
}

void Sequence::Finalize()
{
//    std::cout << __PRETTY_FUNCTION__ << std::endl;
    IInterpreterState::Finalize();
    if (_commands->expressions.size() > 0)
    {
        _bulkmt.eventSequenceComplete.Dispatch(_commands);
    }
}




