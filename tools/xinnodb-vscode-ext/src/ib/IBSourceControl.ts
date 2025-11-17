import { IBCommand } from "./IBCommand";
import { IBDisposable } from "./IBDisposable";
import { IBUri } from "./IBUri";
import { IBSourceControlInputBox } from "./IBSourceControlInputBox";
import { IBSourceControlResourceGroup } from "./IBSourceControlResourceGroup";
import { IBQuickDiffProvider } from "./IBQuickDiffProvider";

/**
 * @brief Represents a source control system in the InnoDB extension.
 */
export interface IBSourceControl extends IBDisposable {
  readonly id: string;
  readonly label: string;
  readonly rootUri: IBUri | undefined;
  readonly inputBox: IBSourceControlInputBox;
  readonly count?: number;
  readonly statusBarCommands?: IBCommand[];
  readonly resourceGroups: IBSourceControlResourceGroup[];
  readonly quickDiffProvider?: IBQuickDiffProvider;
  createResourceGroup(id: string, label: string): IBSourceControlResourceGroup;
  dispose(): void;
}
