/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2007-2012
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTRUSIVE_OPTIONS_HPP
#define BOOST_INTRUSIVE_OPTIONS_HPP

#include <boost/intrusive/detail/config_begin.hpp>
#include <boost/intrusive/intrusive_fwd.hpp>
#include <boost/intrusive/link_mode.hpp>
#include <boost/intrusive/detail/mpl.hpp>
#include <boost/intrusive/detail/utilities.hpp>
#include <boost/intrusive/annotations.hpp>
#include <boost/static_assert.hpp>

#if !defined(BOOST_INTRUSIVE_VARIADIC_TEMPLATES)
#include <boost/intrusive/detail/annotations_compat.hpp>
#endif

namespace boost {
namespace intrusive {

/// @cond

struct default_tag;
struct member_tag;

namespace detail{

struct default_hook_tag{};
struct default_annotations_tag{};

#define BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION(BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER) \
struct BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER : public default_hook_tag\
{\
   template <class T>\
   struct apply\
   {  typedef typename T::BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER type;  };\
}\

BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION(default_list_hook);
BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION(default_slist_hook);
BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION(default_set_hook);
BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION(default_uset_hook);
BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION(default_avl_set_hook);
BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION(default_splay_set_hook);
BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION(default_bs_set_hook);

#undef BOOST_INTRUSIVE_DEFAULT_HOOK_MARKER_DEFINITION

template <class ValueTraits>
struct eval_value_traits
{
   typedef typename ValueTraits::value_traits type;
};

template <class BucketTraits>
struct eval_bucket_traits
{
   typedef typename BucketTraits::bucket_traits type;
};

template <class T, class BaseHook>
struct concrete_hook_base_value_traits
{
   typedef typename BaseHook::boost_intrusive_tags tags;
   typedef detail::base_hook_traits
      < T
      , typename get_annotated_node_traits<tags>::type
      , tags::link_mode
      , typename tags::tag
      , tags::hook_type> type;
};

template <class BaseHook>
struct concrete_hook_base_node_traits
{  typedef typename BaseHook::boost_intrusive_tags::node_traits type;  };

template <class T, class BaseHook>
struct any_hook_base_value_traits
{
   typedef typename BaseHook::boost_intrusive_tags tags;
   typedef detail::base_hook_traits
      < T
      , typename BaseHook::node_traits
      , tags::link_mode
      , typename tags::tag
      , tags::hook_type> type;
};

template <class BaseHook>
struct any_hook_base_node_traits
{  typedef typename BaseHook::node_traits type; };

template<class T, class BaseHook>
struct get_base_value_traits
{
   typedef typename detail::eval_if_c
      < internal_any_hook_bool_is_true<BaseHook>::value
      , any_hook_base_value_traits<T, BaseHook>
      , concrete_hook_base_value_traits<T, BaseHook>
      >::type type;
};

template<class BaseHook>
struct get_base_node_traits
{
   typedef typename detail::eval_if_c
      < internal_any_hook_bool_is_true<BaseHook>::value
      , any_hook_base_node_traits<BaseHook>
      , concrete_hook_base_node_traits<BaseHook>
      >::type type;
};

template<class T, class MemberHook>
struct get_member_value_traits
{
   typedef typename MemberHook::member_value_traits type;
};

template<class MemberHook>
struct get_member_node_traits
{
   typedef typename MemberHook::member_value_traits::node_traits type;
};

template<class T, class SupposedValueTraits>
struct get_value_traits
{
   // SupposedValueTraits is either a value traits class, one of our hooks,
   // or placeholder derrived from default_hook_tag.

   // Base hooks with no Tag option will have default_x_tag defined via
   // make_default_definer. if the user doesn't specify a hook to the container,
   // SupposedValueTraits will be detail::default_x_tag. In this case, use
   // apply to get default_x_hook out of T, yielding the real hook type.
   typedef typename eval_if_c
      < is_convertible<SupposedValueTraits*, default_hook_tag*>::value
      , apply<SupposedValueTraits, T>
      , identity<SupposedValueTraits>
   >::type supposed_value_traits;
   //...supposed_value_traits is either a hook or a custom value traits class.

   typedef typename eval_if_c
      < internal_base_hook_bool_is_true<supposed_value_traits>::value
      //...get it's internal value traits using
      //the provided T value type.
      , get_base_value_traits<T, supposed_value_traits>
      //...else use it's internal value traits tag
      //(member hooks and custom value traits are in this group)
      , eval_if_c
         < internal_member_value_traits<supposed_value_traits>::value
         , get_member_value_traits<T, supposed_value_traits>
         , identity<supposed_value_traits>
         >
      >::type type;
};

template<class ValueTraits>
struct get_real_value_traits
{
   static const bool external_value_traits =
      external_value_traits_bool_is_true<ValueTraits>::value;
   typedef typename eval_if_c
      < external_value_traits
      , eval_value_traits<ValueTraits>
      , identity<ValueTraits>
      >::type type;
};

template<class ValueTraits>
struct get_explicit_node_traits
{
   typedef typename ValueTraits::node_traits type;
};

template<class SupposedValueTraits>
struct get_node_traits
{
   typedef SupposedValueTraits supposed_value_traits;
   //...if it's a base hook
   typedef typename eval_if_c
      < internal_base_hook_bool_is_true<supposed_value_traits>::value
      //...get it's internal value traits using
      //the provided T value type.
      , get_base_node_traits<supposed_value_traits>
      //...else use it's internal value traits tag
      //(member hooks and custom value traits are in this group)
      , eval_if_c
         < internal_member_value_traits<supposed_value_traits>::value
         , get_member_node_traits<supposed_value_traits>
         , get_explicit_node_traits<supposed_value_traits>
         >
      >::type type;
};

template<class RealValueTraits>
struct get_hook_annotations
{
   typedef typename RealValueTraits::annotated_node_traits::annotations type;
};

template<class ValueTraits, class Annotations>
struct get_annotations
{
   typedef typename get_real_value_traits<ValueTraits>::type  real_value_traits;
   // If you fail this static assertion, you're trying to use annotations but
   // your hook/value_traits doesn't support them.
   BOOST_STATIC_ASSERT((
         is_same<Annotations, default_annotations_tag>::value ||
         annotations_supported_is_true<real_value_traits>::value));

   // (Annotations != default_annotations_tag) => Annotations
   // (real_value_traits::annotations_supported != true) => annotations<>
   // otherwise => real_value_traits::annotated_node_traits::annotations
   typedef typename eval_if_c
      < is_same<Annotations, default_annotations_tag>::value
      , eval_if_c
         < annotations_supported_is_true<real_value_traits>::value
         , get_hook_annotations<real_value_traits>
         , identity<annotations<> >
         >
      , identity<Annotations>
      >::type type;
};

template<class Annotation, class Config>
struct make_real_annotation
{
   typedef typename Annotation::template make_real_annotation<Config>::type real_annotation;

   struct type : real_annotation
   {
      static const bool explicit_annotation_hook_type = true;
      typedef Annotation annotation_hook;
   };
};

#if !defined(BOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template<class Config, BOOST_INTRUSIVE_ANNOTATION_TATN>
struct get_real_annotations_impl
{
   typedef annotations
      <  typename eval_if_c
         < apply_annotation_config_is_true<A1>::value
         , make_real_annotation<A1, Config>
         , identity<A1>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A2>::value
         , make_real_annotation<A2, Config>
         , identity<A2>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A3>::value
         , make_real_annotation<A3, Config>
         , identity<A3>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A4>::value
         , make_real_annotation<A4, Config>
         , identity<A4>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A5>::value
         , make_real_annotation<A5, Config>
         , identity<A5>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A6>::value
         , make_real_annotation<A6, Config>
         , identity<A6>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A7>::value
         , make_real_annotation<A7, Config>
         , identity<A7>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A8>::value
         , make_real_annotation<A8, Config>
         , identity<A8>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A9>::value
         , make_real_annotation<A9, Config>
         , identity<A9>
         >::type
      ,  typename eval_if_c
         < apply_annotation_config_is_true<A10>::value
         , make_real_annotation<A10, Config>
         , identity<A10>
         >::type
      > type;
};
#else
template<class Config, class... Annotations>
struct get_real_annotations_impl
{
   typedef annotations
      < typename eval_if_c
         < apply_annotation_config_is_true<Annotations>::value
         , make_real_annotation<Annotations, Config>
         , identity<Annotations>
         >::type...
      > type;
};
#endif

template<class Config, class Annotations>
struct get_real_annotations
{
   typedef typename Annotations::template apply1<get_real_annotations_impl, Config>::type::type type;
};

}  //namespace detail


//!This type indicates that no option is being used
//!and that the default options should be used
struct none
{
    template<class Base>
    struct pack : Base
    { };
};

/// @endcond

//!This option setter specifies if the intrusive
//!container stores its size as a member to
//!obtain constant-time size() member.
template<bool Enabled>
struct constant_time_size
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        static const bool constant_time_size = Enabled;
    };
/// @endcond
};

//!This option setter specifies the type that
//!the container will use to store its size.
template<class SizeType>
struct size_type
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        typedef SizeType size_type;
    };
/// @endcond
};

//!This option setter specifies the strict weak ordering
//!comparison functor for the value type
template<class Compare>
struct compare
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        typedef Compare compare;
    };
/// @endcond
};

//!This option setter for scapegoat containers specifies if
//!the intrusive scapegoat container should use a non-variable
//!alpha value that does not need floating-point operations.
//!
//!If activated, the fixed alpha value is 1/sqrt(2). This
//!option also saves some space in the container since
//!the alpha value and some additional data does not need
//!to be stored in the container.
//!
//!If the user only needs an alpha value near 1/sqrt(2), this
//!option also improves performance since avoids logarithm
//!and division operations when rebalancing the tree.
template<bool Enabled>
struct floating_point
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        static const bool floating_point = Enabled;
    };
/// @endcond
};

//!This option setter specifies the equality
//!functor for the value type
template<class Equal>
struct equal
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        typedef Equal equal;
    };
/// @endcond
};

//!This option setter specifies the equality
//!functor for the value type
template<class Priority>
struct priority
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        typedef Priority priority;
    };
/// @endcond
};

//!This option setter specifies the hash
//!functor for the value type
template<class Hash>
struct hash
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        typedef Hash hash;
    };
/// @endcond
};

//!This option setter specifies the relationship between the type
//!to be managed by the container (the value type) and the node to be
//!used in the node algorithms. It also specifies the linking policy.
template<typename ValueTraits>
struct value_traits
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        typedef ValueTraits value_traits;
    };
/// @endcond
};

//!This option setter specifies the member hook the
//!container must use.
template< typename Parent
        , typename MemberHook
        , MemberHook Parent::* PtrToMember>
struct member_hook
{
/// @cond
   typedef detail::member_hook_traits
      < Parent
      , MemberHook
      , PtrToMember
      > member_value_traits;
   template<class Base>
   struct pack : Base
   {
      typedef member_value_traits value_traits;
   };
/// @endcond
};


//!This option setter specifies the function object that will
//!be used to convert between values to be inserted in a container
//!and the hook to be used for that purpose.
template< typename Functor>
struct function_hook
{
/// @cond
   typedef detail::function_hook_traits
      <Functor> function_value_traits;
   template<class Base>
   struct pack : Base
   {
      typedef function_value_traits value_traits;
   };
/// @endcond
};


//!This option setter specifies that the container
//!must use the specified base hook
template<typename BaseHook>
struct base_hook
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      typedef BaseHook value_traits;
   };
/// @endcond
};

//!This option setter specifies the type of
//!a void pointer. This will instruct the hook
//!to use this type of pointer instead of the
//!default one
template<class VoidPointer>
struct void_pointer
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      typedef VoidPointer void_pointer;
   };
/// @endcond
};

//!This option setter specifies the type of
//!the tag of a base hook. A type cannot have two
//!base hooks of the same type, so a tag can be used
//!to differentiate two base hooks with otherwise same type
template<class Tag>
struct tag
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      typedef Tag tag;
   };
/// @endcond
};

//!This option setter specifies the link mode
//!(normal_link, safe_link or auto_unlink)
template<link_mode_type LinkType>
struct link_mode
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      static const link_mode_type link_mode = LinkType;
   };
/// @endcond
};

//!This option setter specifies if the hook
//!should be optimized for size instead of for speed.
template<bool Enabled>
struct optimize_size
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      static const bool optimize_size = Enabled;
   };
/// @endcond
};

//!This option setter specifies if the list container should
//!use a linear implementation instead of a circular one.
template<bool Enabled>
struct linear
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      static const bool linear = Enabled;
   };
/// @endcond
};

//!This option setter specifies if the list container should
//!use a linear implementation instead of a circular one.
template<bool Enabled>
struct cache_last
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      static const bool cache_last = Enabled;
   };
/// @endcond
};

//!This option setter specifies the bucket traits
//!class for unordered associative containers. When this option is specified,
//!instead of using the default bucket traits, a user defined holder will be defined
template<class BucketTraits>
struct bucket_traits
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      typedef BucketTraits bucket_traits;
   };
/// @endcond
};

//!This option setter specifies if the unordered hook
//!should offer room to store the hash value.
//!Storing the hash in the hook will speed up rehashing
//!processes in applications where rehashing is frequent,
//!rehashing might throw or the value is heavy to hash.
template<bool Enabled>
struct store_hash
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        static const bool store_hash = Enabled;
    };
/// @endcond
};

//!This option setter specifies if the unordered hook
//!should offer room to store another link to another node
//!with the same key.
//!Storing this link will speed up lookups and insertions on
//!unordered_multiset containers with a great number of elements
//!with the same key.
template<bool Enabled>
struct optimize_multikey
{
/// @cond
    template<class Base>
    struct pack : Base
    {
        static const bool optimize_multikey = Enabled;
    };
/// @endcond
};

//!This option setter specifies if the bucket array will be always power of two.
//!This allows using masks instead of the default modulo operation to determine
//!the bucket number from the hash value, leading to better performance.
//!In debug mode, if power of two buckets mode is activated, the bucket length
//!will be checked to through assertions to assure the bucket length is power of two.
template<bool Enabled>
struct power_2_buckets
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      static const bool power_2_buckets = Enabled;
   };
/// @endcond
};

//!This option setter specifies if the container will cache a pointer to the first
//!non-empty bucket so that begin() is always constant-time.
//!This is specially helpful when we can have containers with a few elements
//!but with big bucket arrays (that is, hashtables with low load factors).
template<bool Enabled>
struct cache_begin
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      static const bool cache_begin = Enabled;
   };
/// @endcond
};


//!This option setter specifies if the container will compare the hash value
//!before comparing objects. This option can't be specified if store_hash<>
//!is not true.
//!This is specially helpful when we have containers with a high load factor.
//!and the comparison function is much more expensive that comparing already
//!stored hash values.
template<bool Enabled>
struct compare_hash
{
/// @cond
   template<class Base>
   struct pack : Base
   {
      static const bool compare_hash = Enabled;
   };
/// @endcond
};

//!This option setter specifies if the hash container will use incremental
//!hashing. With incremental hashing the cost of hash table expansion is spread
//!out across each hash table insertion operation, as opposed to be incurred all at once.
//!Therefore linear hashing is well suited for interactive applications or real-time
//!appplications where the worst-case insertion time of non-incremental hash containers
//!(rehashing the whole bucket array) is not admisible.
template<bool Enabled>
struct incremental
{
   /// @cond
   template<class Base>
   struct pack : Base
   {
      static const bool incremental = Enabled;
   };
   /// @endcond
};

#if !defined(BOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template <BOOST_INTRUSIVE_ANNOTATION_TATN>
struct annotations
{
   template <template <BOOST_INTRUSIVE_ANNOTATION_TAT> class T>
   struct apply0
   {
      typedef T<BOOST_INTRUSIVE_ANNOTATION_TAN> type;
   };

   template <template <class,BOOST_INTRUSIVE_ANNOTATION_TAT> class T, class TArg1>
   struct apply1
   {
      typedef T<TArg1,BOOST_INTRUSIVE_ANNOTATION_TAN> type;
   };

   template <template <class,class,BOOST_INTRUSIVE_ANNOTATION_TAT> class T, class TArg1, class TArg2>
   struct apply2
   {
      typedef T<TArg1,TArg2,BOOST_INTRUSIVE_ANNOTATION_TAN> type;
   };

   template<class Base>
   struct pack : Base
   {
      typedef ::boost::intrusive::annotations<BOOST_INTRUSIVE_ANNOTATION_TAN> annotations;
   };
};
#else
template <class ...Annotations>
struct annotations
{
   /* putting this here allows both direct use and use as an option */
   template <template <class ...> class T, class... TArgs>
   struct apply
   {
      typedef T<TArgs..., Annotations...> type;
   };

   /* we can't fake the variadics here, so as long as we support C++03, we have
    * to use apply0/1/2 thoughout the boost::intrusive codebase. These are
    * all implemented in terms of the generic 'apply' in C++11, to ease
    * understanding.
    */
   template <template <class ...> class T, class... TArgs>
   struct apply0 : apply<T,TArgs...> {};
   template <template <class ...> class T, class... TArgs>
   struct apply1 : apply<T,TArgs...> {};
   template <template <class ...> class T, class... TArgs>
   struct apply2 : apply<T,TArgs...> {};

   template<class Base>
   struct pack : Base
   {
      typedef ::boost::intrusive::annotations<Annotations...> annotations;
   };
};
#endif

namespace detail{

struct default_annotations
{
   template<class Base>
   struct pack : Base
   {
      typedef default_annotations_tag annotations;
   };
};

} //namespace detail

/// @cond

//To-do: pass to variadic templates
#if !defined(BOOST_INTRUSIVE_VARIADIC_TEMPLATES)

template<class Prev, class Next>
struct do_pack
{
   //Use "pack" member template to pack options
   typedef typename Next::template pack<Prev> type;
};

template<class Prev>
struct do_pack<Prev, none>
{
   //Avoid packing "none" to shorten template names
   typedef Prev type;
};

template
   < class DefaultOptions
   , class O1         = none
   , class O2         = none
   , class O3         = none
   , class O4         = none
   , class O5         = none
   , class O6         = none
   , class O7         = none
   , class O8         = none
   , class O9         = none
   , class O10        = none
   , class O11        = none
   >
struct pack_options
{
   // join options
   typedef
      typename do_pack
      <  typename do_pack
         <  typename do_pack
            <  typename do_pack
               <  typename do_pack
                  <  typename do_pack
                     <  typename do_pack
                        <  typename do_pack
                           <  typename do_pack
                              <  typename do_pack
                                 <  typename do_pack
                                    < DefaultOptions
                                    , O1
                                    >::type
                                 , O2
                                 >::type
                              , O3
                              >::type
                           , O4
                           >::type
                        , O5
                        >::type
                     , O6
                     >::type
                  , O7
                  >::type
               , O8
               >::type
            , O9
            >::type
         , O10
         >::type
      , O11
      >::type
   type;
};
#else

//index_tuple
template<int... Indexes>
struct index_tuple{};

//build_number_seq
template<std::size_t Num, typename Tuple = index_tuple<> >
struct build_number_seq;

template<std::size_t Num, int... Indexes>
struct build_number_seq<Num, index_tuple<Indexes...> >
   : build_number_seq<Num - 1, index_tuple<Indexes..., sizeof...(Indexes)> >
{};

template<int... Indexes>
struct build_number_seq<0, index_tuple<Indexes...> >
{  typedef index_tuple<Indexes...> type;  };

template<class ...Types>
struct typelist
{};

//invert_typelist
template<class T>
struct invert_typelist;

template<int I, typename Tuple>
struct typelist_element;

template<int I, typename Head, typename... Tail>
struct typelist_element<I, typelist<Head, Tail...> >
{
   typedef typename typelist_element<I-1, typelist<Tail...> >::type type;
};

template<typename Head, typename... Tail>
struct typelist_element<0, typelist<Head, Tail...> >
{
   typedef Head type;
};

template<int ...Ints, class ...Types>
typelist<typename typelist_element<(sizeof...(Types) - 1) - Ints, typelist<Types...> >::type...>
   inverted_typelist(index_tuple<Ints...>, typelist<Types...>)
{
   return typelist<typename typelist_element<(sizeof...(Types) - 1) - Ints, typelist<Types...> >::type...>();
}

//sizeof_typelist
template<class Typelist>
struct sizeof_typelist;

template<class ...Types>
struct sizeof_typelist< typelist<Types...> >
{
   static const std::size_t value = sizeof...(Types);
};

//invert_typelist_impl
template<class Typelist, class Indexes>
struct invert_typelist_impl;


template<class Typelist, int ...Ints>
struct invert_typelist_impl< Typelist, index_tuple<Ints...> >
{
   static const std::size_t last_idx = sizeof_typelist<Typelist>::value - 1;
   typedef typelist
      <typename typelist_element<last_idx - Ints, Typelist>::type...> type;
};

template<class Typelist, int Int>
struct invert_typelist_impl< Typelist, index_tuple<Int> >
{
   typedef Typelist type;
};

template<class Typelist>
struct invert_typelist_impl< Typelist, index_tuple<> >
{
   typedef Typelist type;
};

//invert_typelist
template<class Typelist>
struct invert_typelist;

template<class ...Types>
struct invert_typelist< typelist<Types...> >
{
   typedef typelist<Types...> typelist_t;
   typedef typename build_number_seq<sizeof...(Types)>::type indexes_t;
   typedef typename invert_typelist_impl<typelist_t, indexes_t>::type type;
};

//Do pack
template<class Typelist>
struct do_pack;

template<>
struct do_pack<typelist<> >;

template<class Prev>
struct do_pack<typelist<Prev> >
{
   typedef Prev type;
};

template<class Prev, class Last>
struct do_pack<typelist<Prev, Last> >
{
   typedef typename Prev::template pack<Last> type;
};

template<class Prev, class ...Others>
struct do_pack<typelist<Prev, Others...> >
{
   typedef typename Prev::template pack
      <typename do_pack<typelist<Others...> >::type> type;
};


template<class ...Options>
struct pack_options
{
   typedef typelist<Options...> typelist_t;
   typedef typename invert_typelist<typelist_t>::type inverted_typelist;
   typedef typename do_pack<inverted_typelist>::type type;
};

#endif

struct hook_defaults
   :  public pack_options
      < none
      , void_pointer<void*>
      , link_mode<safe_link>
      , tag<default_tag>
      , optimize_size<false>
      , store_hash<false>
      , linear<false>
      , optimize_multikey<false>
      , annotations<>
      >::type
{};

/// @endcond

}  //namespace intrusive {
}  //namespace boost {

#include <boost/intrusive/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTRUSIVE_OPTIONS_HPP
