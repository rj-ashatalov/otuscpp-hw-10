#pragma once

#include <string>
#include <functional>
#include <map>

template<typename ...Args>
class EventDispatcher
{
        using handler_type = std::function<void(Args...)>;

        struct Subscription
        {
            Subscription(handler_type&& handler)
                    : handler(handler)
            {
            }

            handler_type handler;
        };

    public:
        void Subscribe(handler_type&& handler)
        {
            auto&& subscription = std::make_shared<Subscription>(std::forward<std::remove_reference_t<handler_type>>(handler));
            _subscriptionList.push_back(subscription);
        };

        void UnsubscribeAll()
        {
            _subscriptionList.clear();
        };

        void Dispatch(Args... args)
        {
            for (auto& subscription: _subscriptionList)
            {
                subscription->handler(std::forward<Args>(args)...);
            }
        };
    private:
        std::vector<std::shared_ptr<Subscription>> _subscriptionList;
};


