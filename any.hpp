// Copyright Davydov Nikolay 2022

#ifndef INCLUDE_ANY_HPP_
#define INCLUDE_ANY_HPP_

#include <typeinfo>
#include <new>
#include <utility>
#include <type_traits>
#include <memory>

namespace my_any {
    class any {
        template<typename ValueType>
        friend const ValueType *any_cast(const any *) noexcept;

    public:
        constexpr any() noexcept = default;

        any(const any &other) {
            if (other.instance_) {
                instance_ = other.instance_->clone();
            }
        }

        any(any &&other) noexcept
                : instance_(std::move(other.instance_)) {}

        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<std::decay_t<ValueType>, any>>>
        explicit any(ValueType &&value) {
            emplace<std::decay_t<ValueType>>(std::forward<ValueType>(value));
        }

        any &operator=(const any &rhs) {
            any(rhs).swap(*this);
            return *this;
        }

        any &operator=(any &&rhs) noexcept {
            any(std::move(rhs)).swap(*this);
            return *this;
        }

        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<std::decay_t<ValueType>, any>>>
        any &operator=(ValueType &&rhs) {
            any(std::forward<ValueType>(rhs)).swap(*this);
            return *this;
        }

        template<typename ValueType, typename... Args>
        void emplace(Args &&... args) {
            instance_ = std::make_unique<storage_impl < std::decay_t<ValueType>> >(std::forward<Args>(args)...);
        }

        void reset() noexcept {
            instance_.reset();
        }

        void swap(any &other) noexcept {
            std::swap(instance_, other.instance_);
        }


        [[nodiscard]] bool has_value() const noexcept {
            return static_cast<bool>(instance_);
        }

        [[nodiscard]] const std::type_info &type() const noexcept {
            return instance_ ? instance_->type() : typeid(void);
        }

    private:
        struct storage_base;

        std::unique_ptr<storage_base> instance_;

        struct storage_base {
            virtual ~storage_base() = default;

            [[nodiscard]] virtual const std::type_info &type() const noexcept = 0;

            [[nodiscard]] virtual std::unique_ptr<storage_base> clone() const = 0;
        };

        template<typename ValueType>
        struct storage_impl final : public storage_base {
            template<typename... Args>
            explicit storage_impl(Args &&... args)
                    : value(std::forward<Args>(args)...) {}

            [[nodiscard]] const std::type_info &type() const noexcept override {
                return typeid(ValueType);
            }

            [[nodiscard]] std::unique_ptr<storage_base> clone() const override {
                return std::make_unique<storage_impl < ValueType>>
                (value);
            }

            ValueType value;
        };

    };

    class bad_any_cast : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override {
            return "bad any cast";
        }
    };

    template<typename ValueType>
    ValueType any_cast(const any &anything) {
        using value_type_cvref = std::remove_cv_t<std::remove_reference_t<ValueType>>;
        if (auto *value = any_cast<value_type_cvref>(&anything)) {
            return static_cast<ValueType>(*value);
        } else {
            throw bad_any_cast();
        }
    }

    template<typename ValueType>
    const ValueType *any_cast(const any *anything) noexcept {
        if (!anything) return nullptr;
        auto *storage = dynamic_cast<any::storage_impl<ValueType> *>(anything->instance_.get());
        if (!storage) return nullptr;
        return &storage->value;
    }

}

#endif //INCLUDE_ANY_HPP_
