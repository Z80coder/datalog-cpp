#ifndef TUPLES_H
#define TUPLES_H

#include "variable.h"

namespace datalog {

// TODO: reify the concept of a tuple of values and pointers to Variables

/**
 * @brief unbind a variable
 * 
 * @tparam T 
 * @param t 
 */
template <typename T>
void unbind(Variable<T> *t)
{
    t->unbind();
}

/**
 * @brief unbind no-operation for types that are not variables (i.e. values)
 * 
 * @tparam T 
 * @param t 
 */
template <typename T>
void unbind(const T &t) {
	// If t is not a Variable then perform no-op
}

/**
 * @brief apply unbind to a tuple of variables and values
 * 
 * @tparam Ts 
 * @param tuple 
 */
template <typename... Ts>
void unbind(const tuple<Ts...> &tuple)
{
    apply([](auto &&... args) { ((unbind(args), ...)); }, tuple);
}

}

#endif // TUPLES_H