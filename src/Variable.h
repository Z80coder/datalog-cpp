#ifndef VARIABLE_H
#define VARIABLE_H

#include <tuple>

namespace datalog
{
using namespace std;

/**
 * @brief Represents a datalog variable that is either free or bound to a value.
 * 
 * @tparam T is the type of the variable
 */
template <typename T>
struct Variable : optional<T>
{
    /**
     * @brief bind this variable to the supplied value (making it an unfree variable)
     * 
     * @param value
     */
    void bind(const T &value)
    {
        this->emplace(value);
    }

    /**
     * @brief unbinds this variable (making it a free variable)
     * 
     */
    void unbind()
    {
        this->reset();
    }

    /**
     * @brief checks whether this variable is free or bound
     * 
     * @return true if bound to a value
     * @return false if free
     */
    bool isBound() const
    {
        return this->has_value();
    }

    /**
     * @brief returns the bound value (if not bound then throws an exception)
     * 
     * @return const T& 
     */
    const T &value() const
    {
        return this->optional<T>::value();
    }
};

template <typename T>
Variable<T> *var()
{
    return new Variable<T>();
}

template <typename T>
T val(Variable<T> *t)
{
    return t->value();
}

template <typename T>
void deleteVar(Variable<T> *v)
{
    delete v;
}

template <typename T>
void unbind(Variable<T> *t)
{
    t->unbind();
}

template <typename T>
void unbind(const T &t) {}

template <typename... Ts>
void unbind(const tuple<Ts...> &tuple)
{
    apply([](auto &&... args) { ((unbind(args), ...)); }, tuple);
}

template <typename T>
bool bind(const T &a, const T &b)
{
    return a == b;
}

template <typename T>
bool bind(const T &a, Variable<T> *const b)
{
    if (b->isBound())
    {
        return b->value() == a;
    }
    b->bind(a);
    return true;
}

} // namespace datalog

#endif