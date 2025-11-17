/**
 * @brief Extended terminal options specific to the InnoDB extension.
 */
export interface IBExtensionTerminalOptions {
  autoClose?: boolean;
  splitView?: boolean;
  workspaceEnv?: { [key: string]: string };
}
