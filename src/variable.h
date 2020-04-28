#ifndef VARIABLE_H
#define VARIABLE_H

#include <optional>

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

} // namespace datalog

#endif