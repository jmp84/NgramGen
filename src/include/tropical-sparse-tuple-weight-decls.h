// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use these files except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Copyright 2012 - Gonzalo Iglesias, Adrià de Gispert, William Byrne






#ifndef TROPICALSPARSETUPLEWEIGHTDECLS_H_
#define TROPICALSPARSETUPLEWEIGHTDECLS_H_

/**
 * \file
 * \brief Basic declarations used for tropical sparse vector weight semiring.
 * \date 10-10-2012
 * \author Rory Waite
 */

using namespace fst;
typedef vector<double> PARAMS;

typedef TropicalWeightTpl<double> FeatureWeight;

typedef TropicalSparseTupleWeight<double> TupleW;

typedef ArcTpl<TupleW> TupleArc;

typedef VectorFst<TupleArc> TupleArcFst;

//Define 32 bit versions

typedef TropicalWeightTpl<float> FeatureWeight32;

typedef TropicalSparseTupleWeight<float> TupleW32;

typedef ArcTpl<TupleW32> TupleArc32;

typedef VectorFst<TupleArc32> TupleArcFst32;


#endif /* TROPICALSPARSETUPLEWEIGHTDECLS_H_ */
