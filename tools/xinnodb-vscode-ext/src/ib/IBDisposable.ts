import * as vscode from 'vscode';

/**
 * @brief Disposable resource management for the InnoDB extension.
 *
 * IBDisposable represents a resource that can be disposed of to free up memory
 * and other resources. This is essential for preventing memory leaks in long-running
 * applications like VS Code extensions.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#Disposable}
 *
 * @example
 * ```typescript
 * import { IBDisposable } from './ib';
 *
 * // Create a disposable resource
 * class MyResource implements IBDisposable {
 *   private disposed = false;
 *
 *   dispose(): void {
 *     if (!this.disposed) {
 *       this.disposed = true;
 *       // Clean up resources
 *       this.cleanup();
 *     }
 *   }
 *
 *   private cleanup(): void {
 *     // Release file handles, clear timers, etc.
 *     console.log('Resource cleaned up');
 *   }
 * }
 *
 * // Use the resource
 * const resource = new MyResource();
 *
 * // Dispose when done
 * resource.dispose();
 *
 * // Or use in a try/finally block
 * try {
 *   // Use resource
 * } finally {
 *   resource.dispose();
 * }
 * ```
 */
export interface IBDisposable extends vscode.Disposable {
  /**
   * Dispose of this resource.
   *
   * This method should be called when the resource is no longer needed.
   * It should clean up any resources (file handles, timers, event listeners, etc.)
   * and mark the object as disposed to prevent double disposal.
   *
   * @method dispose
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#Disposable}
   *
   * @example
   * ```typescript
   * class FileWatcher implements IBDisposable {
   *   private disposed = false;
   *
   *   dispose(): void {
   *     if (!this.disposed) {
   *       this.disposed = true;
   *       // Stop watching files
   *       this.stopWatching();
   *     }
   *   }
   * }
   * ```
   */
  dispose(): void;
}

/**
 * @brief Static methods for working with disposables in the InnoDB extension.
 *
 * This namespace provides utility methods for creating and managing disposables.
 *
 * @namespace IBDisposable
 * @since 1.0.0
 */
export const IBDisposable = {
  /**
   * Create a disposable from a function.
   *
   * @param callOnDispose Function to call when disposed.
   * @returns A disposable that calls the function.
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#Disposable}
   *
   * @example
   * ```typescript
   * // Create disposable from function
   * const disposable = IBDisposable.fromFunction(() => {
   *   console.log('Cleaning up...');
   *   clearInterval(timerId);
   * });
   *
   * // Dispose later
   * disposable.dispose();
   * ```
   */
  fromFunction: (callOnDispose: () => void): IBDisposable => ({
    dispose: callOnDispose
  }),

  /**
   * Create a disposable that does nothing.
   *
   * @returns A no-op disposable.
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#Disposable}
   *
   * @example
   * ```typescript
   * // Return no-op disposable when no cleanup needed
   * return IBDisposable.None;
   * ```
   */
  None: Object.freeze({
    dispose: () => {}
  })
};
