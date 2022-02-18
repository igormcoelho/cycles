#pragma once

#include <assert.h>
#include <iostream>
#include <memory>
#include <variant>

template <typename T>
using sptr = std::shared_ptr<T>;
//
template <typename T>
using wptr = std::weak_ptr<T>;