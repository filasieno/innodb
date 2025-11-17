/**
 * @brief Future type for asynchronous operations in the InnoDB extension.
 *
 * IBFuture represents an asynchronous operation that may complete in the future.
 * It provides methods to handle completion, cancellation, and chaining operations.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 */
export interface IBFuture<T> {
  /**
   * Whether the future has completed.
   */
  readonly done: boolean;

  /**
   * The result value if completed successfully.
   */
  readonly result?: T;

  /**
   * Any error that occurred during execution.
   */
  readonly error?: Error;

  /**
   * Cancel the future operation.
   */
  cancel(): void;

  /**
   * Register a callback for when the future completes.
   */
  then<U>(callback: (value: T) => U | IBFuture<U>): IBFuture<U>;

  /**
   * Register a callback for when the future fails.
   */
  catch<U>(callback: (error: Error) => U | IBFuture<U>): IBFuture<U>;
}
