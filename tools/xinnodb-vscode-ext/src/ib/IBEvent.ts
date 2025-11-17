import * as vscode from 'vscode';

/**
 * @brief Event system for the InnoDB extension.
 *
 * IBEvent provides a type-safe event system that allows components to communicate
 * asynchronously. Events can have listeners that are called when the event is fired,
 * and listeners can be disposed to prevent memory leaks.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#Event}
 *
 * @example
 * ```typescript
 * import { IBEvent } from './ib';
 *
 * // Define an event
 * const onDidChange: IBEvent<string> = (listener, thisArgs?, disposables?) => {
 *   // Store listener and return disposable
 *   const disposable = {
 *     dispose: () => {
 *       // Remove listener
 *       removeListener(listener);
 *     }
 *   };
 *
 *   // Add listener
 *   addListener(listener);
 *
 *   // Add to disposables if provided
 *   if (disposables) {
 *     disposables.push(disposable);
 *   }
 *
 *   return disposable;
 * };
 *
 * // Listen to the event
 * const disposable = onDidChange((value) => {
 *   console.log('Value changed to:', value);
 * });
 *
 * // Fire the event (internal implementation)
 * fireEvent('new value');
 *
 * // Dispose when done
 * disposable.dispose();
 * ```
 */
export interface IBEvent<T> extends vscode.Event<T> {
  /**
   * Register a listener for this event.
   *
   * @param listener The function to call when the event fires.
   * @param thisArgs The value to use as `this` when calling the listener.
   * @param disposables An array to which the disposable will be added.
   * @returns A disposable that unregisters the listener.
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#Event}
   *
   * @example
   * ```typescript
   * // Simple listener
   * const disposable = event((value) => {
   *   console.log('Event fired with:', value);
   * });
   *
   * // Listener with custom this context
   * const disposable = event.call(myObject, (value) => {
   *   console.log('Event fired with:', value);
   *   console.log('this.name:', this.name);
   * });
   *
   * // Add to disposables array
   * const disposables: vscode.Disposable[] = [];
   * const disposable = event((value) => {
   *   console.log('Event fired');
   * }, undefined, disposables);
   *
   * // All disposables can be disposed at once
   * vscode.Disposable.from(...disposables).dispose();
   * ```
   */
  (listener: (e: T) => any, thisArgs?: any, disposables?: vscode.Disposable[]): vscode.Disposable;
}
