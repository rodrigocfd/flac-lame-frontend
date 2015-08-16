/*!
 * @file
 * @brief Vector utilities.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <vector>

namespace wolf {
namespace vec {

template<typename T> inline void Append(std::vector<T>& v, const std::vector<T>& other) { v.insert(v.end(), other.begin(), other.end()); }
template<typename T> inline void Remove(std::vector<T>& v, int idx)                     { v.erase(std::next(v.begin(), idx)); }
template<typename T> inline void Remove(std::vector<T>& v, int idxFirst, int idxLast)   { v.erase(std::next(v.begin(), idxFirst), std::next(v.begin(), idxLast + 1)); }
template<typename T> void ReposElem(std::vector<T>& v, size_t idx, size_t toIdx) {
	if (idx == toIdx || idx >= v.size() || toIdx >= v.size()) return;
	std::swap(v[idx], v[toIdx]);
	if (idx < toIdx) { for (size_t i = idx; i <= toIdx - 2; ++i) std::swap(v[i], v[i + 1]); }
	else { for (size_t i = idx; i >= toIdx + 2; --i) std::swap(v[i], v[i - 1]); }
}

}//namespace vec
}//namespace wolf
