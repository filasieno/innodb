import { IBUri } from './IBUri';
import { IBFileSystemProvider } from './IBFileSystemProvider';

/**
 * @brief Virtual workspace for the InnoDB extension.
 */
export interface IBVirtualWorkspace {
  readonly name: string;
  readonly rootUri: IBUri;
  readonly fileSystems: IBFileSystemProvider[];
  resolveUri(uri: string | IBUri): IBUri;
  containsUri(uri: IBUri): boolean;
}
