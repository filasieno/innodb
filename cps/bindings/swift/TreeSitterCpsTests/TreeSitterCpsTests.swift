import XCTest
import SwiftTreeSitter
import TreeSitterCps

final class TreeSitterCpsTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_cps())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading cps C grammar")
    }
}
