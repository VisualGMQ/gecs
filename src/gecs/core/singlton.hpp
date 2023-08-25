#pragma once

#include <memory>

namespace gecs {

template <typename T, bool ExplicitInit, typename Loader>
class singlton;

/**
 * @brief abstract singlton class for convienient implement Singlton Pattern
 *
 * this singlton will init singlton automatically when call instance();
 *
 * @tparam T
 * @tparam Loader a function object to point out how to create the singlton
 */
template <typename T, typename Loader>
class singlton<T, false, Loader> {
public:
    virtual ~singlton() = default;

    static T& instance() {
        static T instance = Loader{}();
        return instance;
    }
};

/**
 * @brief abstract singlton class for convienient implement Singlton Pattern
 *
 * this singlton must call init() to create instance before use instance(), and
 * release singlton call destroy() after use
 *
 * @tparam T
 * @tparam Loader a function object to point out how to create the singlton
 */

template <typename T, typename Loader>
class singlton<T, true, Loader> {
public:
    virtual ~singlton() = default;

    static T& instance() {
        GECS_ASSERT(instance_, "instance not init, you must call T::init() before get this "
                   "singlton");
        return *instance;
    }

    template <typename... Args>
    static void init(Args&&... args) {
        instance_ = std::make_unique<T>(Loader{}(std::forward<Args>(args)...));
    }

    static void destroy() { instance_.reset(); }

private:
    inline static std::unique_ptr<T> instance_;
};

}  // namespace gecs