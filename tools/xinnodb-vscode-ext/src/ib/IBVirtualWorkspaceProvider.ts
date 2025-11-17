import { IBUri } from './IBUri';
import { IBVirtualWorkspace } from './IBVirtualWorkspace';

/**
 * @brief Provider for virtual workspaces in the InnoDB extension.
 */
export interface IBVirtualWorkspaceProvider {
  createWorkspace(name: string, rootUri: IBUri): IBVirtualWorkspace;
  getWorkspaces(): IBVirtualWorkspace[];
  findWorkspace(uri: IBUri): IBVirtualWorkspace | undefined;
}
