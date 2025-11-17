import { IBCommand } from "./IBCommand";
import { IBDisposable } from "./IBDisposable";
import { IBSourceControlResourceState } from "./IBSourceControlResourceState";

/**
 * @brief Represents a group of source control resources in the InnoDB extension.
 */
export interface IBSourceControlResourceGroup extends IBDisposable {
  id: string;
  label: string;
  hideWhenEmpty?: boolean;
  resourceStates: IBSourceControlResourceState[];
  command?: IBCommand;
}
