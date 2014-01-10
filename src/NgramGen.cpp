/*
 ============================================================================
 Name        : NgramGen.cpp
 Author      : Juan Pino
 Version     :
 Copyright   : DWTFYW
 Description : Ngram generation
 ============================================================================
 */

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "Decoder.h"

DEFINE_string(sentence_file, "",
              "Name of a file containing sentences to be reordered");
DEFINE_string(ngrams, "", "Name of a directory containing ngram and coverage "
              "files applicable to the input words");
DEFINE_string(lm, "", "Language model file directory, in arpa or kenlm format.");
DEFINE_string(fstoutput, "", "Directory name for the fst outputs.");
DEFINE_string(range, "1", "Range of items to be processed");
DEFINE_int32(overlap, 0, "Maximum overlap allowed when extending a state.");
DEFINE_int32(prune_nbest, 0, "N-best pruning: number of states kept in a"
             " column");
DEFINE_int32(prune_nbest_input_length_specific, 0,
             "N-best pruning dependent on the input length: the pruning "
             "option given by the user is divided by the sentence length, for "
             "example if the option given is 10000 and the input length is "
             "10, the effective n-best pruning is 10000/10 = 1000.");
DEFINE_double(prune_threshold, 0, "Threshold pruning: add this threshold to "
              " the lowest cost in a column to define what states are kept.");
DEFINE_double(dump_prune, 0, "Pruning parameter for dumping lattices. If set to"
              " a value greater than 0, then apply fstprune --weight= before"
              " writing.");
DEFINE_bool(add_input, false, "If true, add the input sentence to the output "
            "lattice, this ensures that we at least regenerate the input.");
DEFINE_bool(when_lost_input, false, "If true, looks for when the input was lost"
            " as a hypothesis");
DEFINE_string(features, "", "Comma separated list of features");
DEFINE_string(weights, "", "Comma separated list of feature weights. "
              "The format is featureName1=weight1,featureName2=weight2 etc.");
DEFINE_string(task, "decode", "Task: either 'decode' or 'tune'. If the task is "
              "decode, the output is a StdVectorFst. If the task is tune, then "
              "the output is a fst with sparse weights.");
DEFINE_string(chop, "", "Defines the chopping strategy. Either 'silly' (chops "
              "every n word where n is defined by --max_chop) or 'punctuation' "
              "(chops after each punctuation or n word where punctuation is "
              "defined by --punctuation and n is defined by --max_chop; "
              "--wordmap needs also to be defined to check if a word id is a "
              "punctuation symbol). By default, no chopping is done.");
DEFINE_int32(max_chop, 0, "For choppers that use a max size for each chop.");
DEFINE_string(
    punctuation, "", "Punctuation file with one punctuation symbol per line. "
        "Used for the punctuation chopper.");
DEFINE_string(wordmap, "", "Word map. Used for the punctuation chopper to "
    "check if a word id is a punctuation symbol.");
DEFINE_string(chop_file, "", "File with chopping info. The format is one "
    "chopping info per line. Each line is space separated integers. Each "
    "integer indicates the zero-based index of the first word of the next "
    "chunk.");
DEFINE_string(constraints, "", "Defines the constraints strategy. Only 'chunk' "
    "is supported. By default, no constraints are imposed.");
DEFINE_string(constraints_file, "", "File with constraints. The constraints are"
    " one per line. Each line is a bit string. A one indicates that the chunk "
    "may be reordered. A zero indicates that the chunk cannot be reordered. "
    "Chunks are currently defined by chopping.");
DEFINE_bool(allow_deletion, false, "Allows unigrams to be deleted (an epsilon "
    "arc will be added to the resulting fst).");

namespace cam {
namespace eng {
namespace gen {

void checkArgs(int argc, char** argv) {
  std::string usage = "Generates a lattice of reordered sentences.\n\n Usage: ";
  usage += argv[0];
  usage += "argv[0] --sentenceFile=sentenceFile --ngrams=ngramDirectory "
      "--lm=lmDirectory --fstoutput=fstoutputDirectory "
      "[--prune_nbest=<integer> | --prune_threshold=<double>]\n";
  google::ParseCommandLineFlags(&argc, &argv, true);
  CHECK_NE("", FLAGS_sentence_file) << "Missing input --sentence_file" <<
      std::endl << usage;
  CHECK_NE("", FLAGS_ngrams) << "Missing ngrams --ngram" << std::endl << usage;
  CHECK_NE("", FLAGS_lm) << "Missing language model directory --lm" <<
      std::endl << usage;
  CHECK_NE("", FLAGS_fstoutput) << "Missing output directory --fstoutput" <<
      std::endl << usage;
  CHECK((FLAGS_prune_nbest == 0 && FLAGS_prune_threshold == 0) ||
        (FLAGS_prune_nbest != 0 && FLAGS_prune_threshold == 0) ||
        (FLAGS_prune_nbest == 0 && FLAGS_prune_threshold != 0)) << "Only one "
            "threshold strategy is allowed: --prune_nbest or --prune_threshold";
  CHECK(FLAGS_task == "decode" || FLAGS_task == "tune") << "Unknown task: " <<
      FLAGS_task << ". The task can only be 'decode' or 'tune'";
  // TODO check all flags
  // TODO check for length dependent pruning
}

} // namespace gen
} // namespace eng
} // namespace cam

/**
 * Main function. Decodes several sentences from an input file.
 */
int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  using namespace cam::eng::gen;
  checkArgs(argc, argv);
  Decoder decoder(
      FLAGS_sentence_file, FLAGS_ngrams, FLAGS_lm, FLAGS_fstoutput,
      FLAGS_range, FLAGS_overlap, FLAGS_prune_nbest,
      FLAGS_prune_nbest_input_length_specific, FLAGS_prune_threshold,
      FLAGS_dump_prune, FLAGS_add_input, FLAGS_when_lost_input, FLAGS_features,
      FLAGS_weights, FLAGS_task, FLAGS_chop, FLAGS_max_chop, FLAGS_punctuation,
      FLAGS_wordmap, FLAGS_chop_file, FLAGS_constraints,
      FLAGS_constraints_file, FLAGS_allow_deletion);
  decoder.decode();
}
