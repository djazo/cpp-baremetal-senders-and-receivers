#pragma once

#include <async/completes_synchronously.hpp>
#include <async/completion_tags.hpp>
#include <async/concepts.hpp>
#include <async/connect.hpp>
#include <async/debug.hpp>
#include <async/env.hpp>
#include <async/type_traits.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

namespace async {
namespace _just_result_of {
template <stdx::ct_string Name, typename Tag, typename R, typename... Fs>
struct op_state : Fs... {
    template <typename F>
    using has_void_result = std::is_void<std::invoke_result_t<F>>;

    [[no_unique_address]] R receiver;

    auto start() & -> void {
        debug_signal<"start", debug::erased_context_for<op_state>>(
            get_env(receiver));
        using split_returns =
            boost::mp11::mp_partition<boost::mp11::mp_list<Fs...>,
                                      has_void_result>;

        [&]<typename... Ts>(boost::mp11::mp_list<Ts...>) {
            (static_cast<Ts &>(*this)(), ...);
        }(boost::mp11::mp_front<split_returns>{});

        [&]<typename... Ts>(boost::mp11::mp_list<Ts...>) {
            debug_signal<Tag::name, debug::erased_context_for<op_state>>(
                get_env(receiver));
            Tag{}(std::move(receiver), static_cast<Ts &>(*this)()...);
        }(boost::mp11::mp_back<split_returns>{});
    }

    [[nodiscard]] constexpr auto query(get_env_t) const noexcept {
        return prop{completes_synchronously_t{}, std::true_type{}};
    }
};

template <stdx::ct_string Name, typename Tag, std::invocable... Fs>
struct sender : Fs... {
    template <receiver R>
    [[nodiscard]] constexpr auto
    connect(R &&r) && -> op_state<Name, Tag, std::remove_cvref_t<R>, Fs...> {
        check_connect<sender &&, R>();
        return {{static_cast<Fs &&>(std::move(*this))}..., std::forward<R>(r)};
    }

    template <receiver R>
        requires(... and std::copy_constructible<Fs>)
    [[nodiscard]] constexpr auto connect(
        R &&r) const & -> op_state<Name, Tag, std::remove_cvref_t<R>, Fs...> {
        check_connect<sender const &, R>();
        return {{static_cast<Fs const &>(*this)}..., std::forward<R>(r)};
    }

    template <typename... Ts>
    using make_signature = async::completion_signatures<Tag(Ts...)>;

    using is_sender = void;
    using completion_signatures = boost::mp11::mp_apply<
        make_signature,
        boost::mp11::mp_copy_if_q<
            async::completion_signatures<std::invoke_result_t<Fs>...>,
            boost::mp11::mp_not_fn<std::is_void>>>;

    [[nodiscard]] constexpr auto query(get_env_t) const noexcept {
        return prop{completes_synchronously_t{}, std::true_type{}};
    }
};
} // namespace _just_result_of

template <stdx::ct_string Name = "just_result_of", std::invocable... Fs>
[[nodiscard]] constexpr auto just_result_of(Fs &&...fs) -> sender auto {
    return _just_result_of::sender<Name, set_value_t,
                                   std::remove_cvref_t<Fs>...>{
        std::forward<Fs>(fs)...};
}

template <stdx::ct_string Name = "just_error_result_of", std::invocable... Fs>
[[nodiscard]] constexpr auto just_error_result_of(Fs &&...fs) -> sender auto {
    return _just_result_of::sender<Name, set_error_t,
                                   std::remove_cvref_t<Fs>...>{
        std::forward<Fs>(fs)...};
}

struct just_result_of_t;
struct just_error_result_of_t;

template <typename Tag>
using just_result_of_tag_for =
    stdx::conditional_t<std::same_as<Tag, set_value_t>, just_result_of_t,
                        just_error_result_of_t>;

template <stdx::ct_string Name, typename Tag, typename R, typename... Fs>
struct debug::context_for<_just_result_of::op_state<Name, Tag, R, Fs...>> {
    using tag = async::just_result_of_tag_for<Tag>;
    constexpr static auto name = Name;
    using children = stdx::type_list<>;
    using type = _just_result_of::op_state<Name, Tag, R, Fs...>;
};
} // namespace async
