package tree_sitter_cps_test

import (
	"testing"

	tree_sitter "github.com/tree-sitter/go-tree-sitter"
	tree_sitter_cps "github.com/tree-sitter/tree-sitter-cps/bindings/go"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_cps.Language())
	if language == nil {
		t.Errorf("Error loading cps C grammar")
	}
}
