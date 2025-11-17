import * as vscode from 'vscode';

/**
 * @fileoverview InnoDB terminal dimensions interface
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @interface IBTerminalDimensions
 * @extends vscode.TerminalDimensions
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#TerminalDimensions}
 * @example
 * @brief ```typescript
 * const dimensions: IBTerminalDimensions = {
 *   columns: 80,
 *   rows: 24
 * };
 * ```
 */

/**
 * @brief Represents the dimensions (size) of a terminal in the InnoDB extension.
 *
 * This interface extends vscode.TerminalDimensions and defines the width
 * and height of a terminal in terms of character columns and rows. Terminal
 * dimensions are used to determine the layout and display area available
 * for terminal output and input.
 *
 * The dimensions represent the number of character cells that can be
 * displayed horizontally (columns) and vertically (rows) in the terminal.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @extends vscode.TerminalDimensions
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#TerminalDimensions}
 * @example
 * ```typescript
 * import { IBTerminalDimensions } from './ib';
 *
 * // Standard terminal size (80x24)
 * const standardTerminal: IBTerminalDimensions = {
 *   columns: 80,
 *   rows: 24
 * };
 *
 * // Wide terminal for development
 * const wideTerminal: IBTerminalDimensions = {
 *   columns: 120,
 *   rows: 30
 * };
 *
 * // Compact terminal for limited space
 * const compactTerminal: IBTerminalDimensions = {
 *   columns: 60,
 *   rows: 15
 * };
 *
 * // Use in pseudoterminal implementation
 * class MyPseudoterminal implements IBPseudoterminal {
 *   open(initialDimensions: IBTerminalDimensions): void {
 *     console.log(`Terminal opened with ${initialDimensions.columns}x${initialDimensions.rows}`);
 *     this.resize(initialDimensions);
 *   }
 *
 *   setDimensions(dimensions: IBTerminalDimensions): void {
 *     // Adjust internal buffer size
 *     this.buffer.resize(dimensions.columns, dimensions.rows);
 *   }
 * }
 * ```
 */
export interface IBTerminalDimensions extends vscode.TerminalDimensions {
  /**
   * The number of character columns in the terminal.
   *
   * This represents the width of the terminal in terms of the number
   * of character cells that can be displayed horizontally. Each column
   * typically displays one character, though some characters may span
   * multiple columns (e.g., wide characters in Unicode).
   *
   * Common values include 80 (standard terminal width), 120 (wide terminal),
   * or 40 (narrow terminal for side panels).
   *
   * @property {number} columns
   * @readonly
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#TerminalDimensions}
   * @example
   * ```typescript
   * // Check if terminal is wide enough for side-by-side output
   * if (dimensions.columns >= 120) {
   *   displaySideBySide();
   * } else {
   *   displayStacked();
   * }
   *
   * // Calculate total character capacity
   * const totalChars = dimensions.columns * dimensions.rows;
   * ```
   */
  readonly columns: number;

  /**
   * The number of character rows in the terminal.
   *
   * This represents the height of the terminal in terms of the number
   * of lines that can be displayed vertically. Each row typically
   * displays one line of text, though some content may span multiple
   * rows or use only partial rows.
   *
   * Common values include 24 (standard terminal height), 30 (tall terminal),
   * or 10 (short terminal for compact displays).
   *
   * @property {number} rows
   * @readonly
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#TerminalDimensions}
   * @example
   * ```typescript
   * // Check if terminal has enough height for complex output
   * if (dimensions.rows >= 20) {
   *   displayFullInterface();
   * } else {
   *   displayCompactInterface();
   * }
   *
   * // Calculate viewport area
   * const area = dimensions.columns * dimensions.rows;
   * console.log(`Terminal area: ${area} character cells`);
   * ```
   */
  readonly rows: number;
}
