/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Jeff Snyder    2010-2011
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTRUSIVE_DETAIL_ANNOTATION_ALGORITHMS_HPP
#define BOOST_INTRUSIVE_DETAIL_ANNOTATION_ALGORITHMS_HPP

#include <boost/mpl/has_xxx.hpp>
#include <boost/mpl/identity.hpp>

#include <boost/intrusive/options.hpp>

namespace boost {
namespace intrusive {
namespace detail {

template <class T>
struct eval_annotation_hook
{
   typedef typename T::annotation_hook type;
};

template <class T>
struct get_annotation_hook
{
   typedef typename eval_if_c
      < explicit_annotation_hook_is_true<T>::value
      , eval_annotation_hook<T>
      , identity<T>
      >::type type;
};

template <class T>
class tree_algorithms;

template <class AnnotatedNodeTraits, class Annotation>
struct annotation_algorithms
{
   typedef typename AnnotatedNodeTraits::annotation_list_ptr        annotation_list_ptr;
   typedef typename AnnotatedNodeTraits::const_annotation_list_ptr  const_annotation_list_ptr;
   typedef typename get_annotation_hook<Annotation>::type           annotation_hook;

   static void update(annotation_list_ptr n){
      Annotation::template update_node<AnnotatedNodeTraits>(n);
   }

   static void clone_node(const_annotation_list_ptr source, annotation_list_ptr target){
      AnnotatedNodeTraits::template set_annotation_value<annotation_hook>(target, AnnotatedNodeTraits::template get_annotation_value<annotation_hook>(source));
   }
};

#if !defined(BOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template <class NodeTraits, BOOST_INTRUSIVE_ANNOTATION_TATND>
struct annotated_node_algorithms;
#else
template <class NodeTraits, class... Annotations>
struct annotated_node_algorithms;
#endif

/*
Specialize the no-annotation case to avoid iterating to the top of the tree for update_to_top
*/
#if !defined(BOOST_INTRUSIVE_VARIADIC_TEMPLATES)
// the base case for C++03 is where all annotations are no_annotation
template <class AnnotatedNodeTraits>
struct annotated_node_algorithms<AnnotatedNodeTraits,BOOST_INTRUSIVE_ANNOTATION_TAD>
#else
template <class AnnotatedNodeTraits>
struct annotated_node_algorithms<AnnotatedNodeTraits>
#endif
{
   typedef typename AnnotatedNodeTraits::node_ptr        node_ptr;
   typedef typename AnnotatedNodeTraits::const_node_ptr  const_node_ptr;

   static void update(node_ptr){}
   static void clone_node(const_node_ptr source, node_ptr target){}
   static void on_left_rotation(const_node_ptr){}
   static void on_right_rotation(const_node_ptr){}
   static void update_to_top(node_ptr){}
   static void update_to_top(const_node_ptr, node_ptr){}
};

#if !defined(BOOST_INTRUSIVE_VARIADIC_TEMPLATES)
// the ..._TAN_SHIFT macro drops the first annotation and puts no_annotation on the end
template <class AnnotatedNodeTraits, BOOST_INTRUSIVE_ANNOTATION_TATN>
struct annotated_node_algorithms : private annotated_node_algorithms<AnnotatedNodeTraits, BOOST_INTRUSIVE_ANNOTATION_TAN_SHIFT>
#else
template <class AnnotatedNodeTraits, class AnnotationsHead, class... AnnotationsTail>
struct annotated_node_algorithms<AnnotatedNodeTraits, AnnotationsHead, AnnotationsTail...> : private annotated_node_algorithms<AnnotatedNodeTraits, AnnotationsTail...>
#endif
{
private:
#if !defined(BOOST_INTRUSIVE_VARIADIC_TEMPLATES)
   typedef A1                                                                  AnnotationsHead;
   typedef annotated_node_algorithms<AnnotatedNodeTraits, BOOST_INTRUSIVE_ANNOTATION_TAN_SHIFT>
                                                                               list_tail_algorithms;
#else
   typedef annotated_node_algorithms<AnnotatedNodeTraits, AnnotationsTail...>  list_tail_algorithms;
#endif
   typedef typename AnnotatedNodeTraits::node_traits                            node_traits;
   typedef node_tree_algorithms<node_traits>                                    base_tree_algorithms;

public:
   typedef typename AnnotatedNodeTraits::node_ptr        node_ptr;
   typedef typename AnnotatedNodeTraits::const_node_ptr  const_node_ptr;

   static void update(node_ptr n){
      annotation_algorithms<AnnotatedNodeTraits, AnnotationsHead>::update(AnnotatedNodeTraits::to_annotation_list_ptr(n));
      list_tail_algorithms::update(n);
   };

   static void clone_node(const_node_ptr source, node_ptr target){
      annotation_algorithms<AnnotatedNodeTraits, AnnotationsHead>::clone_node(AnnotatedNodeTraits::to_annotation_list_ptr(source), AnnotatedNodeTraits::to_annotation_list_ptr(target));
      list_tail_algorithms::clone_node(source, target);
   }

   static void on_left_rotation(const_node_ptr pivot){
      update_to_top(pivot);
   }

   static void on_right_rotation(const_node_ptr pivot){
      update_to_top(pivot);
   }

   static void update_to_top(node_ptr n){
      while(!base_tree_algorithms::is_header(n)) {
         update(n);
         n = node_traits::get_parent(n);
      }
   }

   static void update_to_top(const_node_ptr header, node_ptr n){
      while(n != header) {
         update(n);
         n = node_traits::get_parent(n);
      }
   }
};

template <class AnnotatedNodeTraits, class Annotations>
struct make_annotated_node_algorithms
{
   typedef typename Annotations::template apply1<
         annotated_node_algorithms,
         AnnotatedNodeTraits
      >::type type;
};

}
}
}

#endif // BOOST_INTRUSIVE_DETAIL_ANNOTATION_ALGORITHMS_HPP

// vim: set sw=3 sts=3 ts=3 et:
