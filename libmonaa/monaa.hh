#pragma once

/*! 
  @file monaa.hh

  @mainpage Reference Manual of libmonaa
  
  This is the reference manual of libmonaa. For documentation on how to install libmonaa, see the <a href="https://github.com/MasWag/monaa/blob/master/libmonaa/README.md">README</a>.

  @section Overview

  This library mainly provides the function @link monaa @endlink, which does do online timed pattern matching by the timed FJS algorithm, and as its parameters, the classes @link WordContainer @endlink, @link TimedAutomaton @endlink, and @link AnsContainer @endlink. The input and output container classes @link WordContainer @endlink and @link AnsContainer @endlink just define the interface of the container, and the classes passed by their template arguments defines the procedure. Therefore, users can define the functionality suitable for their application. For example, @link AnsContainer::push_back() @endlink can be used as a call back function when the procedure finds a matching in the input.

  @note This document is not completed yet. The content may be changed later.
  @copyright (C) 2017 Masaki Waga. All rights reserved.
*/ 


#include "word_container.hh"
#include "zone_container.hh"
#include "timed_automaton.hh"

/*!
  @brief Execute the timed FJS algorithm.
  @param [in] word A container of a timed word representing a log.
  @param [in] A A timed automaton used as a pattern.
  @param [out] ans A container for the answer zone.
*/
void monaa(AbstractTimedWordContainer &word,
           const TimedAutomaton &A,
           AbstractAnsZoneContainer &ans);

/*!
  @brief Execute the timed FJS algorithm.
  @param [in] word A container of a timed word representing a log.
  @param [in] A A timed automaton used as a pattern.
  @param [out] ans A container for the answer zone.
*/
void monaaDollar(AbstractTimedWordContainer &word,
                 const TimedAutomaton &A,
                 AbstractAnsZoneContainer &ans);
