#ifndef ENTT_CORE_ALGORITHM_HPP
#define ENTT_CORE_ALGORITHM_HPP


#include <functional>
#include <algorithm>
#include <utility>
#include "utility.hpp"


namespace entt {


/**
 * @brief Function object to wrap `std::sort` in a class type.
 *
 * Unfortunately, `std::sort` cannot be passed as template argument to a class
 * template or a function template.<br/>
 * This class fills the gap by wrapping some flavors of `std::sort` in a
 * function object.
 */
struct std_sort {
    /**
     * @brief Sorts the elements in a range.
     *
     * Sorts the elements in a range using the given binary comparison function.
     *
     * @tparam It Type of random access iterator.
     * @tparam Compare Type of comparison function object.
     * @tparam Args Types of arguments to forward to the sort function.
     * @param first An iterator to the first element of the range to sort.
     * @param last An iterator past the last element of the range to sort.
     * @param compare A valid comparison function object.
     * @param args Arguments to forward to the sort function, if any.
     */
    template<typename It, typename Compare = std::less<>, typename... Args>
    void operator()(It first, It last, Compare compare = Compare{}, Args &&... args) const {
        std::sort(std::forward<Args>(args)..., std::move(first), std::move(last), std::move(compare));
    }
};


/*! @brief Function object for performing insertion sort. */
struct insertion_sort {
    /**
     * @brief Sorts the elements in a range.
     *
     * Sorts the elements in a range using the given binary comparison function.
     *
     * @tparam It Type of random access iterator.
     * @tparam Compare Type of comparison function object.
     * @param first An iterator to the first element of the range to sort.
     * @param last An iterator past the last element of the range to sort.
     * @param compare A valid comparison function object.
     */
    template<typename It, typename Compare = std::less<>>
    void operator()(It first, It last, Compare compare = Compare{}) const {
        if(first < last) {
            for(auto it = first+1; it < last; ++it) {
                auto value = std::move(*it);
                auto pre = it;

                for(; pre > first && compare(value, *(pre-1)); --pre) {
                    *pre = std::move(*(pre-1));
                }

                *pre = std::move(value);
            }
        }
    }
};


/**
 * @brief Function object for performing LSD radix sort.
 * @tparam Bit Number of bits processed per pass.
 * @tparam N Maximum number of bits to sort.
 */
template<std::size_t Pass, std::size_t N>
class radix_sort {
    static_assert((N % Pass) == 0);

    template<typename In, typename Out, typename Getter>
    void sort(In first, In last, Out out_begin, Getter getter = Getter{}, uint32_t pass = 0) const {
        uint32_t start_bit = pass * Pass;

        constexpr int n_buckets = 1 << Pass;
        int bucket_count[n_buckets] = {0};
        constexpr int bit_mask = (1 << Pass) - 1;

        for(auto it = first; it < last; ++it) {
            int bucket = (getter(*it) >> start_bit) & bit_mask;
            ++bucket_count[bucket];
        }

        int out_index[n_buckets];
        out_index[0] = 0;
        for (int i = 1; i < n_buckets; ++i)
            out_index[i] = out_index[i - 1] + bucket_count[i - 1];

        for(auto it = first; it < last; ++it) {
            int bucket = (getter(*it) >> start_bit) & bit_mask;
            out_begin[out_index[bucket]++] = *it;
        }
    }

public:
    /**
     * @brief Sorts the elements in a range.
     *
     * Sorts the elements in a range using the given _getter_ to access the
     * actual data to be sorted.
     *
     * This implementation is inspired by the online book
     * [Physically Based Rendering](http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html#RadixSort).
     *
     * @tparam It Type of random access iterator.
     * @tparam Getter Type of _getter_ function object.
     * @param first An iterator to the first element of the range to sort.
     * @param last An iterator past the last element of the range to sort.
     * @param getter A valid _getter_ function object.
     */
    template<typename It, typename Getter = identity>
    void operator()(It first, It last, Getter getter = Getter{}) const {
        if(first < last) {
            using size_type = typename std::iterator_traits<It>::value_type;
            std::vector<size_type> aux(std::distance(first, last));
            constexpr uint32_t n_passes = N / Pass;

            for (size_t pass = 0; pass < n_passes; ++pass) {
                if (!(pass & 1)) {
                    sort(first, last, aux.begin(), getter, pass);
                } else {
                    sort(aux.begin(), aux.end(), first, getter, pass);
                }
            }

            // Move final result from _aux_ vector, if needed
            if constexpr (n_passes & 1) {
                auto it = first;

                for(auto &v : aux) {
                    *(it++) = std::move(v);
                }
            }

        }
    }
};


}


#endif // ENTT_CORE_ALGORITHM_HPP
