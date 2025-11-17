import { IBCommand } from "./IBCommand";
import { IBUri } from "./IBUri";
import { IBSourceControlResourceDecorations } from "./IBSourceControlResourceDecorations";

/**
 * @brief Represents the state of a resource in the source control system.
 */
export interface IBSourceControlResourceState {
  resourceUri: IBUri;
  command?: IBCommand;
  decorations?: IBSourceControlResourceDecorations;
  contextValue?: string;
  selected?: boolean;
}
