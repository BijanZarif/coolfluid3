// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Solver_Actions_Proto_BlockAccumulator_hpp
#define CF_Solver_Actions_Proto_BlockAccumulator_hpp

#include<iostream>

#include <boost/proto/proto.hpp>

#include "Math/MatrixTypes.hpp"

#include "Solver/CEigenLSS.hpp"

#include "Terminals.hpp"
#include "Transforms.hpp"

/// @file
/// System matrix block accumultation. Current prototype uses dense a dense Eigen matrix and is purely for proof-of-concept

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Tag for system matrix
struct SystemMatrixTag
{
};

/// Indicate that we want to operate on the system matrix of a linear system
template<Uint I, typename T, typename IdxT>
inline typename boost::proto::result_of::make_expr< boost::proto::tag::function, SystemMatrixTag, StoredReference<Solver::CEigenLSS>, MeshTerm<I, T> const &, IdxT >::type const
system_matrix(Solver::CEigenLSS& lss, MeshTerm<I, T> const& var, IdxT local_idx)
{
  return boost::proto::make_expr<boost::proto::tag::function>( SystemMatrixTag(), store(lss), boost::ref(var), local_idx );
}

/// Overload for scalars
template<Uint I, typename T>
inline typename boost::proto::result_of::make_expr< boost::proto::tag::function, SystemMatrixTag, StoredReference<Solver::CEigenLSS>, MeshTerm<I, T> const &, const Uint >::type const
system_matrix(Solver::CEigenLSS& lss, MeshTerm<I, T> const& var)
{
  return boost::proto::make_expr<boost::proto::tag::function>( SystemMatrixTag(), store(lss), boost::ref(var), 0u );
}
  
/// Tag for RHS
struct SystemRHSTag
{
};
  
/// Indicate that we want to operate on the RHS of a linear system
template<Uint I, typename T, typename IdxT>
inline typename boost::proto::result_of::make_expr< boost::proto::tag::function, SystemRHSTag, StoredReference<Solver::CEigenLSS>, MeshTerm<I, T> const &, IdxT >::type const
system_rhs(Solver::CEigenLSS& lss, MeshTerm<I, T> const& var, IdxT local_idx)
{
  return boost::proto::make_expr<boost::proto::tag::function>( SystemRHSTag(), store(lss), boost::ref(var), local_idx );
}

/// Overload for scalars
template<Uint I, typename T>
inline typename boost::proto::result_of::make_expr< boost::proto::tag::function, SystemRHSTag, StoredReference<Solver::CEigenLSS>, MeshTerm<I, T> const &, const Uint >::type const
system_rhs(Solver::CEigenLSS& lss, MeshTerm<I, T> const& var)
{
  return boost::proto::make_expr<boost::proto::tag::function>( SystemRHSTag(), store(lss), boost::ref(var), 0u );
}
  
/// Helper struct for assignment to a matrix or RHS
template<typename SystemTagT, typename OpTagT>
struct BlockAssignmentOp;

/// Macro to easily define =, +=, -=, ...
#define MAKE_ASSIGN_OP_SYSTEM(__tagname, __op)    \
template<> \
struct BlockAssignmentOp<SystemMatrixTag, boost::proto::tag::__tagname> \
{ \
  template<typename ElementMatrixT, typename DataT> \
  static void assign(Solver::CEigenLSS& lss, const ElementMatrixT& rhs, const DataT& data, const std::vector<Uint>& offsets, const Uint var_idx, const Uint local_idx) \
  { \
    const Mesh::CTable<Uint>::ConstRow connectivity = data.element_connectivity(); \
    const Uint offset = offsets[var_idx]; \
    const Uint total_width = offsets.back(); \
    for(Uint i = 0; i != ElementMatrixT::RowsAtCompileTime; ++i) \
    { \
      const Uint i_idx = connectivity[i]*total_width + offset + local_idx; \
      for(Uint j = 0; j != ElementMatrixT::ColsAtCompileTime; ++j) \
        lss.at(i_idx, connectivity[j]*total_width + offset + local_idx) __op rhs(i, j); \
    } \
  } \
}; \
\
template<> \
struct MatrixAssignOpsCases::case_<boost::proto::tag::__tagname> : \
  boost::proto::__tagname< boost::proto::function< boost::proto::terminal<SystemMatrixTag>, boost::proto::terminal< StoredReference<Solver::CEigenLSS> >, FieldTypes, Integers >, boost::proto::_ > \
{};

/// Allowed block assignment operations
struct MatrixAssignOpsCases
{
  template<typename TagT> struct case_  : boost::proto::not_<boost::proto::_> {};
};

MAKE_ASSIGN_OP_SYSTEM(assign, =)
MAKE_ASSIGN_OP_SYSTEM(plus_assign, +=)
MAKE_ASSIGN_OP_SYSTEM(minus_assign, -=)


//#undef MAKE_ASSIGN_OP_SYSTEM

/// Macro to easily define =, +=, -=, ...
#define MAKE_ASSIGN_OP_RHS(__tagname, __op)    \
template<> \
struct BlockAssignmentOp<SystemRHSTag, boost::proto::tag::__tagname> \
{ \
  template<typename ElementVectorT, typename DataT> \
  static void assign(Solver::CEigenLSS& lss, const ElementVectorT& elem_rhs_contrib, const DataT& data, const std::vector<Uint>& offsets, const Uint var_idx, const Uint local_idx) \
  { \
    static const Uint rhs_dim = ElementVectorT::RowsAtCompileTime == 1 ? ElementVectorT::ColsAtCompileTime : ElementVectorT::RowsAtCompileTime; \
    const typename Eigen::internal::eval<ElementVectorT>::type r = elem_rhs_contrib.eval(); \
    const Mesh::CTable<Uint>::ConstRow connectivity = data.element_connectivity(); \
    const Uint offset = offsets[var_idx]; \
    const Uint total_width = offsets.back(); \
    for(Uint i = 0; i != rhs_dim; ++i) \
      lss.rhs()[connectivity[i]*total_width + offset + local_idx] __op r[i]; \
  } \
}; \
\
template<> \
struct RHSAssignOpsCases::case_<boost::proto::tag::__tagname> : \
  boost::proto::__tagname< boost::proto::function< boost::proto::terminal< SystemRHSTag >, boost::proto::terminal< StoredReference<Solver::CEigenLSS> >, FieldTypes, Integers >, boost::proto::_ > \
{};

/// Allowed RHS assignment operations
struct RHSAssignOpsCases
{
  template<typename TagT> struct case_  : boost::proto::not_<boost::proto::_> {};
};

MAKE_ASSIGN_OP_RHS(assign, =)
MAKE_ASSIGN_OP_RHS(plus_assign, +=)
MAKE_ASSIGN_OP_RHS(minus_assign, -=)

#undef MAKE_ASSIGN_OP_RHS

/// Primitive transform to handle assignment to an LSS matrix
struct BlockAccumulator :
  boost::proto::transform< BlockAccumulator >
{
  template<typename ExprT, typename State, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, State, DataT>
  {
    /// Contrary to general C++ assignment, assigning to a LSS doesn't return anything
    typedef void result_type;

    /// Tag type represents the actual operation type
    typedef typename boost::proto::tag_of<ExprT>::type OpT;
    
    /// Tag indicating if we are modifying the RHS or the system matrix (stored in child 0 of the LHS of the assignment op)
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value
        <
          typename boost::proto::result_of::child_c< typename boost::proto::result_of::left<ExprT>::type, 0 >::type
        >::type
      >::type
    >::type SystemTagT;

    result_type operator ()(
                typename impl::expr_param expr // The assignment expression
              , typename impl::state_param state // should be the element matrix, i.e. RHS already evaluated
              , typename impl::data_param data // data associated with element loop
    ) const
    {
      Solver::CEigenLSS& lss = boost::proto::value( boost::proto::child_c<1>( boost::proto::left(expr) ) ).get();
      const Uint local_idx = boost::proto::value( boost::proto::child_c<3>( boost::proto::left(expr) ) );
      BlockAssignmentOp<SystemTagT, OpT>::assign
      (
        lss,
        state,
        NumberedData()(boost::proto::child_c<2>( boost::proto::left(expr) ), state, data),
        data.variable_offsets(),
        VarNumber()(boost::proto::child_c<2>( boost::proto::left(expr) ), state, data),
        local_idx
      );
    }
  };
};

/// Grammar matching block accumulation expressions
template<typename GrammarT>
struct BlockAccumulation : 
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::switch_<MatrixAssignOpsCases>,
      BlockAccumulator( boost::proto::_expr, GrammarT(boost::proto::_right) )
    >,
    boost::proto::when
    <
      boost::proto::switch_<RHSAssignOpsCases>,
      BlockAccumulator( boost::proto::_expr, GrammarT(boost::proto::_right) )
    >
  >
{
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_BlockAccumulator_hpp
