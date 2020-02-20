#include "Sequence.h"
#include "Bulkmlt.h"
#include <iostream>
#include <ctime>

Sequence::Sequence(Bulkmlt& bulkmlt)
        : IInterpreterState(bulkmlt)
{

}

void Sequence::Exec(std::string ctx)
{
    if (ctx == "{")
    {
        _bulkmlt.SetState<InfinitSequence>();
        return;
    }

    auto command = std::make_shared<Command>();
    command->value = ctx;
    _commands->expressions.push_back(command);

    if (_commands->expressions.size() == 1)
    {
        _bulkmlt.eventFirstCommand.Dispatch(std::time(nullptr));
    }

    if (_commands->expressions.size() >= static_cast<size_t>(_bulkmlt.commandBufCount))
    {
        _bulkmlt.SetState<Sequence>();
    }
}

void Sequence::Initialize()
{
    IInterpreterState::Initialize();
    _commands.reset(new Group());
}

void Sequence::Finalize()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    IInterpreterState::Finalize();
    if (_commands->expressions.size() > 0)
    {
        _bulkmlt.eventSequenceComplete.Dispatch(_commands);
    }
}




