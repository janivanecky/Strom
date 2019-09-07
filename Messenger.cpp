#include "Messenger.h"

void Messaging::RegisterSystem(Messenger *messenger, System *system, MessageHandler *handler)
{
	uint32_t systemCounter = messenger->systemCounter++;
	messenger->systems[systemCounter] = system;
	messenger->handlers[systemCounter] = handler;
	system->messenger = messenger;
}

void Messaging::Dispatch(Messenger *messenger, Message message, void *data)
{
	for (uint32_t i = 0; i < messenger->systemCounter; ++i)
	{
		messenger->handlers[i](messenger->systems[i], message, data);
	}
}