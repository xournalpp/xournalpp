#pragma once

#include <memory>

// taken from https://stackoverflow.com/questions/17902405/how-to-implement-make-unique-function-in-c11
// https://herbsutter.com/gotw/_102/
// Todo: remove with stdc++ >= 14
//       namespace mem = std;

namespace mem
{
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}  // namespace mem
