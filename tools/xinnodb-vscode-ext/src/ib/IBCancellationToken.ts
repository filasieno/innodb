import * as vscode from 'vscode';

/**
 * @brief Cancellation token for asynchronous operations in the InnoDB extension.
 *
 * IBCancellationToken allows long-running operations to be cancelled gracefully.
 * It provides a way to signal that an operation should stop and clean up resources.
 * This is essential for responsive user interfaces where users might cancel operations
 * or when VS Code shuts down while operations are in progress.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#CancellationToken}
 *
 * @example
 * ```typescript
 * import { IBCancellationToken } from './ib';
 *
 * // Use in async operations
 * async function longRunningOperation(token: IBCancellationToken): Promise<void> {
 *   for (let i = 0; i < 100; i++) {
 *     // Check if operation was cancelled
 *     if (token.isCancellationRequested) {
 *       console.log('Operation cancelled');
 *       return;
 *     }
 *
 *     // Do some work
 *     await performStep(i);
 *
 *     // Optional: Check cancellation again after async operation
 *     token.onCancellationRequested(() => {
 *       console.log('Cancellation requested during async operation');
 *     });
 *   }
 * }
 *
 * // Call with cancellation token
 * const tokenSource = new vscode.CancellationTokenSource();
 * longRunningOperation(tokenSource.token);
 *
 * // Cancel after 5 seconds
 * setTimeout(() => tokenSource.cancel(), 5000);
 * ```
 */
export interface IBCancellationToken extends vscode.CancellationToken {
  /**
   * Whether cancellation has been requested.
   *
   * This property should be checked regularly during long-running operations.
   * Once true, the operation should clean up and return as soon as possible.
   *
   * @property {boolean} isCancellationRequested
   * @readonly
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#CancellationToken}
   *
   * @example
   * ```typescript
   * while (!token.isCancellationRequested && hasMoreWork()) {
   *   processNextItem();
   * }
   *
   * if (token.isCancellationRequested) {
   *   cleanupResources();
   * }
   * ```
   */
  readonly isCancellationRequested: boolean;

  /**
   * An event emitted when cancellation is requested.
   *
   * This event allows operations to react immediately to cancellation requests
   * rather than polling the isCancellationRequested property.
   *
   * @property {vscode.Event<void>} onCancellationRequested
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#CancellationToken}
   *
   * @example
   * ```typescript
   * // Register cleanup handler
   * const disposable = token.onCancellationRequested(() => {
   *   console.log('Operation cancelled, cleaning up...');
   *   cleanupResources();
   * });
   *
   * // Don't forget to dispose the listener
   * disposable.dispose();
   * ```
   */
  readonly onCancellationRequested: vscode.Event<void>;
}

/**
 * @brief Source for cancellation tokens in the InnoDB extension.
 *
 * IBCancellationTokenSource creates and manages cancellation tokens.
 * It provides the ability to request cancellation and dispose of resources.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#CancellationTokenSource}
 *
 * @example
 * ```typescript
 * import { IBCancellationTokenSource } from './ib';
 *
 * // Create a token source
 * const tokenSource = new vscode.CancellationTokenSource();
 *
 * // Pass token to operation
 * startLongOperation(tokenSource.token);
 *
 * // Cancel after timeout or user action
 * setTimeout(() => {
 *   tokenSource.cancel();
 *   tokenSource.dispose();
 * }, 30000);
 * ```
 */
export interface IBCancellationTokenSource extends vscode.CancellationTokenSource {
  /**
   * The cancellation token from this source.
   *
   * @property {IBCancellationToken} token
   * @readonly
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#CancellationTokenSource}
   */
  readonly token: IBCancellationToken;

  /**
   * Request cancellation of the token.
   *
   * This will set isCancellationRequested to true and fire the onCancellationRequested event.
   *
   * @method cancel
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#CancellationTokenSource}
   *
   * @example
   * ```typescript
   * // Cancel immediately
   * tokenSource.cancel();
   *
   * // Or cancel after some condition
   * if (userClickedCancel) {
   *   tokenSource.cancel();
   * }
   * ```
   */
  cancel(): void;

  /**
   * Dispose of the token source and its resources.
   *
   * This should be called when the token source is no longer needed
   * to prevent memory leaks.
   *
   * @method dispose
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#CancellationTokenSource}
   *
   * @example
   * ```typescript
   * // Always dispose when done
   * tokenSource.dispose();
   *
   * // In async contexts, use try/finally
   * try {
   *   await longOperation(tokenSource.token);
   * } finally {
   *   tokenSource.dispose();
   * }
   * ```
   */
  dispose(): void;
}
