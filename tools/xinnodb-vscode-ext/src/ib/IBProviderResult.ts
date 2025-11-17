/**
 * @brief Provider result type for the InnoDB extension.
 *
 * This type represents the result of a provider operation, which can be
 * either a value or undefined (when cancelled or not available).
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 */
export type IBProviderResult<T> = T | undefined;
