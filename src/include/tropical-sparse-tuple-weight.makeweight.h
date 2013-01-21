/*
 * tropical-sparse-tuple-weight.makeweight.h
 *
 *  Created on: 18 Jan 2013
 *      Author: jmp84
 */

#ifndef TROPICALSPARSETUPLEWEIGHT_MAKEWEIGHT_H_
#define TROPICALSPARSETUPLEWEIGHT_MAKEWEIGHT_H_

/**
 * Convenience functors that create weights from a float depending on the Arc.
 */
namespace fst
{

/**
 * \brief Templated functor that creates a weight given a float.
 */
template<class Arc = StdArc>
struct  MakeWeight {
  typename Arc::Weight operator()(const float weight) const {
    return typename Arc::Weight(weight);
  }
};

///Template specialization of functor MakeWeight for TupleArc32.
template<>
struct MakeWeight<TupleArc32> {
  TupleArc32::Weight operator()(const float weight) const {
    TropicalSparseTupleWeight<float> result;
    // Warning, the index is one because the 0 index is reserved by
    // TropicalSparseTupleWeight
    result.Push(1, weight);
    return result;
  }
};



} //namespace fst

#endif /* TROPICALSPARSETUPLEWEIGHT_MAKEWEIGHT_H_ */
