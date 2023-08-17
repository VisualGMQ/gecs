#pragma once

namespace gecs {

template <typename T, bool ExplicitInit, typename Loader>
class singlton;

template <typename T, typename Loader>
class singlton<T, false, Loader> {
public:
    virtual ~singlton() = default;

    static T& instance() {
        static T instance = Loader{}();
        return instance;
    }
};

template <typename T, typename Loader>
class singlton<T, true, Loader> {
public:
    virtual ~singlton() = default;

    static T& instance() {
        ECS_ASSERT(instance_);
        return *instance;
    }

    template <typename... Args>
    static void init(Args&&... args) {
        instance_ = std::make_unique<T>(Loader{}(std::forward<Args>(args)...));
    }

    static void destroy() {
        instance_.reset();
    }

private:
    inline static std::unique_ptr<T> instance_;
};

}