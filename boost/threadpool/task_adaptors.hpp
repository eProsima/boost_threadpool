/*! \file
* \brief Task adaptors.
*
* This file contains adaptors for task function objects.
*
* Copyright (c) 2005-2007 Philipp Henkel
* Copyright (c) 2016 Mikhail Komarov
*
* Use, modification, and distribution are  subject to the
* Boost Software License, Version 1.0. (See accompanying  file
* LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*
* http://threadpool.sourceforge.net
*
*/

#ifndef THREADPOOL_TASK_ADAPTERS_HPP_INCLUDED
#define THREADPOOL_TASK_ADAPTERS_HPP_INCLUDED

#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

namespace boost {
    namespace threadpool {

        /*! \brief Standard task function object.
        *
        * This function object wraps a nullary function which returns void.
        * The wrapped function is invoked by calling the operator ().
        *
        * \see boost function library
        *
        */

        template<typename Signature>
        class task_functor {
            template<
                    template<template<typename> class Task, typename TaskReturnType> class SchedulingPolicy,
                    template<typename> class SizePolicy,
                    template<typename> class SizePolicyController,
                    template<typename> class ShutdownPolicy,
                    template<typename> class Task
            >
            friend
            class thread_pool;

        public:
            task_functor(function<Signature(void)> const &input_function = []() -> Signature {
                return Signature();
            })
                    : task_function(input_function) {

            }

            bool empty() {
                return this->task_function.empty();
            }

            Signature operator()(void) const {
                if (!this->task_function.empty()) {
                    return this->task_function();
                }
            }

        protected:
            function<Signature(void)> task_function;
        };

        /*! \brief Prioritized task function object.
        *
        * This function object wraps a task_functor object and binds a priority to it.
        * prio_task_funcs can be compared using the operator < which realises a partial ordering.
        * The wrapped task function is invoked by calling the operator ().
        *
        * \see priority_scheduler
        *
        */
        template<typename Signature>
        class priority_task_functor : public task_functor<Signature> {
        public:
            /*! Constructor.
            * \param priority The priority of the task.
            * \param function The task's function object.
            */
            priority_task_functor(unsigned int const priority, function<Signature(void)> const &input_function = []() -> Signature {
                return Signature();
            })
                    : task_functor<Signature>(input_function),
                      m_priority(priority) {
            }

            template<typename U, typename = typename disable_if_c<is_same<U, Signature>::value, U>::type>
            priority_task_functor(const priority_task_functor<U> &input_reference)
                    : task_functor<Signature>(),
                      m_priority(input_reference.m_priority) {

            }

            template<typename U, typename = typename disable_if_c<is_same<U, Signature>::value, U>::type>
            priority_task_functor &operator=(const priority_task_functor<U> &input_reference) {
                this->m_priority = input_reference.m_priority;
                return *this;
            };

            /*! Executes the task function.
            */
            Signature operator()(void) const {
                if (!this->task_function.empty()) {
                    return this->task_function();
                }
            }

            /*! Comparison operator which realises a partial ordering based on priorities.
            * \param rhs The object to compare with.
            * \return true if the priority of *this is less than right hand side's priority, false otherwise.
            */
            bool operator<(const priority_task_functor &rhs) const {
                return m_priority < rhs.m_priority;
            }

            unsigned int m_priority;  //!< The priority of the task's function.

        };  // priority_task_functor

        /*! \brief Looped task function object.
        *
        * This function object wraps a boolean thread function object.
        * The wrapped task function is invoked by calling the operator () and it is executed in regular
        * time intervals until false is returned. The interval length may be zero.
        * Please note that a pool's thread is engaged as long as the task is looped.
        *
        */
        template<typename Signature>
        class looped_task_functor : public task_functor<Signature> {
        public:
            /*! Constructor.
            * \param function The task's function object which is looped until false is returned.
            * \param interval The minimum break time in milli seconds before the first execution of the task function and between the following ones.
            */
            looped_task_functor(function<Signature(void)> const &input_function = []() -> Signature {
                return Signature();
            }, unsigned int const interval = 0)
                    : task_functor<Signature>(input_function) {
                m_break_s = interval / 1000;
                m_break_ns = (interval - m_break_s * 1000) * 1000 * 1000;
            }

            template<typename U, typename = typename disable_if_c<is_same<U, Signature>::value, U>::type>
            looped_task_functor(const looped_task_functor<U> &input_reference)
                    : task_functor<Signature>(),
                      m_break_s(input_reference.m_break_s),
                      m_break_ns(input_reference.m_break_ns) {

            };

            template<typename U, typename = typename disable_if_c<is_same<U, Signature>::value, U>::type>
            looped_task_functor &operator=(const looped_task_functor<U> &input_reference) {
                this->m_break_ns = input_reference.m_break_ns;
                this->m_break_s = input_reference.m_break_s;
                return *this;
            }

            /*! Executes the task function.
            */
            void operator()(void) const {
                if (!this->task_function.empty()) {
                    if (m_break_s > 0 || m_break_ns >
                                         0) { // Sleep some time before first execution
                        xtime xt;
                        xtime_get(&xt, TIME_UTC_);
                        xt.nsec += m_break_ns;
                        xt.sec += m_break_s;
                        thread::sleep(xt);
                    }

                    while (this->task_function()) {
                        if (m_break_s > 0 || m_break_ns > 0) {
                            xtime xt;
                            xtime_get(&xt, TIME_UTC_);
                            xt.nsec += m_break_ns;
                            xt.sec += m_break_s;
                            thread::sleep(xt);
                        } else {
                            thread::yield(); // Be fair to other threads
                        }
                    }
                }
            }

            unsigned int m_break_s;              //!< Duration of breaks in seconds.
            unsigned int m_break_ns;             //!< Duration of breaks in nano seconds.

        }; // looped_task_functor


    }
} // namespace boost::threadpool

#endif // THREADPOOL_TASK_ADAPTERS_HPP_INCLUDED

