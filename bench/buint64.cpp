

#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#define _SILENCE_CXX20_CISO646_REMOVED_WARNING
#include "tsl/robin_map.h"
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/core/detail/splitmix64.hpp>
#include <boost/config.hpp>
#include "martinus/robin_hood.h"
#include "martinus/unordered_dense.h"
#include "phmap/phmap.h"
#include "hash_table7.hpp"
#include "hash_table6.hpp"
#include "hash_table5.hpp"
#include "hash_table8.hpp"
#include "emilib/emilib2s.hpp"
#include "emilib/emilib2o.hpp"

#include "util.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std::chrono_literals;
#if TKey == 0
using KeyType = uint64_t;
using ValType = uint64_t;
#else
using KeyType = uint32_t;
using ValType = uint32_t;
#endif

static void print_time( std::chrono::steady_clock::time_point & t1, char const* label, uint64_t s, std::size_t size )
{
    auto t2 = std::chrono::steady_clock::now();

    std::cout << label << ": " << ( t2 - t1 ) / 1ms << " ms (s=" << s << ", size=" << size << ")\n";

    t1 = t2;
}

static unsigned N = 2'000'000;
static int K = 10;

static std::vector< KeyType > indices1, indices2, indices3;

static void init_indices()
{
    indices1.push_back( 0 );

    for( unsigned i = 1; i <= N*2; ++i )
    {
        indices1.push_back( i );
    }

    indices2.push_back( 0 );

    {
        boost::detail::splitmix64 rng;

        for( unsigned i = 1; i <= N*2; ++i )
        {
            indices2.push_back( rng() );
        }
    }

    indices3.push_back( 0 );

    for( unsigned i = 1; i <= N*2; ++i )
    {
        indices3.push_back( boost::endian::endian_reverse( static_cast<std::uint64_t>( i ) ) );
//        if (sizeof (KeyType) == sizeof (uint64_t))
//            indices3.push_back( (KeyType)i << 40 );
//        else
//            indices3.push_back( (KeyType)i << 11 );
    }
}

template<class Map>  void test_insert( Map& map, std::chrono::steady_clock::time_point & t1 )
{
    for( unsigned i = 1; i <= N; ++i )
    {
        map.insert( { indices1[ i ], i } );
    }

    print_time( t1, "Consecutive insert",  0, map.size() );

    for( unsigned i = 1; i <= N; ++i )
    {
        map.emplace(  (KeyType)indices2[ i ], (ValType)i  );
    }

    print_time( t1, "Random insert",  0, map.size() );

    for( unsigned i = 1; i <= N; ++i )
    {
        map.insert( { indices3[ i ], i } );
    }

    print_time( t1, "Consecutive shifted insert",  0, map.size() );

    std::cout << std::endl;
}

template<class Map>  void test_lookup( Map& map, std::chrono::steady_clock::time_point & t1 )
{
    std::uint64_t s;

    s = 0;

    for( int j = 0; j < K; ++j )
    {
        for( unsigned i = 1; i <= N * 2; ++i )
        {
//            auto it = map.find( indices1[ i ] );
//            if( it != map.end() ) s += it->second;
            s += map.count(indices1[ i ]);
        }
    }

    print_time( t1, "Consecutive lookup",  s, map.size() );

    s = 0;

    for( int j = 0; j < K; ++j )
    {
        for( unsigned i = 1; i <= N * 2; ++i )
        {
//            auto it = map.find( indices2[ i ] );
//            if( it != map.end() ) s += it->second;
            s += map.count(indices2[ i ]);
        }
    }

    print_time( t1, "Random lookup",  s, map.size() );

    s = 0;

    for( int j = 0; j < K; ++j )
    {
        for( unsigned i = 1; i <= N * 2; ++i )
        {
            auto it = map.find( indices3[ i ] );
            if( it != map.end() ) s += it->second;
        }
    }

    print_time( t1, "Consecutive shifted lookup",  s, map.size() );

    std::cout << std::endl;
}

template<class Map>  void test_iteration( Map& map, std::chrono::steady_clock::time_point & t1 )
{
    auto it = map.begin();

    while( it != map.end() )
    {
        if( it->second & 1 )
        {
            if constexpr( std::is_void_v< decltype( map.erase( it ) ) > )
            {
                map.erase( it++ );
            }
            else
            {
                it = map.erase( it );
            }
        }
        else
        {
            ++it;
        }
    }

    print_time( t1, "Iterate and erase odd elements",  0, map.size() );

    std::cout << std::endl;
}

template<class Map>  void test_erase( Map& map, std::chrono::steady_clock::time_point & t1 )
{
    for( unsigned i = 1; i <= N; ++i )
    {
        map.erase( indices1[ i ] );
    }

    print_time( t1, "Consecutive erase",  0, map.size() );

    for( unsigned i = 1; i <= N; ++i )
    {
        map.erase( indices2[ i ] );
    }

    print_time( t1, "Random erase",  0, map.size() );

    for( unsigned i = 1; i <= N; ++i )
    {
        map.erase( indices3[ i ] );
    }
    print_time( t1, "Consecutive shifted erase",  0, map.size() );

    std::cout << std::endl;
}

// counting allocator

static std::size_t s_alloc_bytes = 0;
static std::size_t s_alloc_count = 0;

template<class T> struct allocator
{
    using value_type = T;

    allocator() = default;

    template<class U> allocator( allocator<U> const & ) noexcept
    {
    }

    template<class U> bool operator==( allocator<U> const & ) const noexcept
    {
        return true;
    }

    template<class U> bool operator!=( allocator<U> const& ) const noexcept
    {
        return false;
    }

    T* allocate( std::size_t n ) const
    {
        s_alloc_bytes += n * sizeof(T);
        s_alloc_count++;

        return std::allocator<T>().allocate( n );
    }

    void deallocate( T* p, std::size_t n ) const noexcept
    {
        s_alloc_bytes -= n * sizeof(T);
        s_alloc_count--;

        std::allocator<T>().deallocate( p, n );
    }
};

//

struct record
{
    std::string label_;
    long long time_;
    std::size_t bytes_;
    std::size_t count_;
};

static std::vector<record> times;

template<template<class...> class Map>  void test( char const* label )
{
    std::cout << label << ":\n\n";

    s_alloc_bytes = 0;
    s_alloc_count = 0;

    Map<KeyType, ValType> map;

    auto t0 = std::chrono::steady_clock::now();
    auto t1 = t0;

    test_insert( map, t1 );

    std::cout << "Memory: " << s_alloc_bytes << " bytes in " << s_alloc_count << " allocations\n\n";

    record rec = { label, 0, s_alloc_bytes, s_alloc_count };

    test_lookup( map, t1 );
    test_iteration( map, t1 );
    test_lookup( map, t1 );
    test_erase( map, t1 );

    auto tN = std::chrono::steady_clock::now();
    std::cout << "Total: " << ( tN - t0 ) / 1ms << " ms\n\n";

    rec.time_ = ( tN - t0 ) / 1ms;
    times.push_back( rec );
}

// aliases using the counting allocator
#if ABSL_HASH
    #define BintHasher absl::Hash<K>
#elif BOOST_HASH
    #define BintHasher boost::hash<K>
#elif FIB_HASH
    #define BintHasher Int64Hasher<K>
#elif ROBIN_HASH
    #define BintHasher robin_hood::hash<K>
#elif CXX17
    #define BintHasher ankerl::unordered_dense::hash<K>
#else
    #define BintHasher std::hash<K>
#endif

template<class K, class V> using allocator_for = ::allocator< std::pair<K const, V> >;

template<class K, class V> using boost_unordered_flat_map =
    boost::unordered_flat_map<K, V, BintHasher, std::equal_to<K>, allocator_for<K, V>>;

template<class K, class V> using std_unordered_map =
    std::unordered_map<K, V, BintHasher, std::equal_to<K>, allocator_for<K, V>>;

template<class K, class V> using emhash_map5 = emhash5::HashMap<K, V, BintHasher, std::equal_to<K>>;
template<class K, class V> using emhash_map6 = emhash6::HashMap<K, V, BintHasher, std::equal_to<K>>;
template<class K, class V> using emhash_map7 = emhash7::HashMap<K, V, BintHasher, std::equal_to<K>>;
template<class K, class V> using emhash_map8 = emhash8::HashMap<K, V, BintHasher, std::equal_to<K>>;

template<class K, class V> using martinus_flat = robin_hood::unordered_map<K, V, BintHasher, std::equal_to<K>>;
template<class K, class V> using emilib_map2 = emilib2::HashMap<K, V, BintHasher, std::equal_to<K>>;
template<class K, class V> using emilib_map3 = emilib3::HashMap<K, V, BintHasher, std::equal_to<K>>;

#ifdef CXX20
template<class K, class V> using jg_densemap = jg::dense_hash_map<K, V, BintHasher, std::equal_to<K>>;
#endif
template<class K, class V> using martinus_dense = ankerl::unordered_dense::map<K, V, BintHasher, std::equal_to<K>>;
template<class K, class V> using phmap_flat  = phmap::flat_hash_map<K, V, BintHasher, std::equal_to<K>>;
template<class K, class V> using tsl_robin_map= tsl::robin_map<K, V, BintHasher, std::equal_to<K>>;

#if ABSL_HMAP
template<class K, class V> using absl_flat_hash_map = absl::flat_hash_map<K, V, BintHasher>;
#endif


int main(int argc, const char* argv[])
{
    if (argc > 1 && isdigit(argv[1][0]))
        N = atoi(argv[1]);
    if (argc > 2 && isdigit(argv[2][0]))
        K = atoi(argv[2]);

    init_indices();


#if ABSL_HMAP
    test<absl_flat_hash_map>("absl::flat_hash_map" );
#endif

//    test<std_unordered_map> ("std::unordered_map" );

#ifdef CXX20
    test<jg_densemap> ("jg_densemap" );
#endif
    test<emhash_map8> ("emhash_map8" );
    test<martinus_dense>("martinus_dense" );
    test<boost_unordered_flat_map>( "boost::unordered_flat_map" );

    test<tsl_robin_map> ("tsl_robin_map" );
    test<phmap_flat> ("phmap_flat" );

    test<emhash_map5> ("emhash_map5" );
    test<emhash_map6> ("emhash_map6" );
    test<emhash_map7> ("emhash_map7" );
    test<martinus_flat> ("martinus_flat" );
    test<emilib_map2> ("emilib_map2" );
    test<emilib_map3> ("emilib_map3" );

    std::cout << "---\n\n";

    for( auto const& x: times )
    {
        std::cout << std::setw( 27 ) << ( x.label_ + ": " ) << std::setw( 5 ) << x.time_ << " ms, " << std::setw( 9 ) << x.bytes_ << " bytes in " << x.count_ << " allocations\n";
    }

    return 0;
}
