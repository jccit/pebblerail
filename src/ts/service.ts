import { config } from "./config";
import { TrainServiceDetails } from "./types/service";

export async function getServiceInfo(
  serviceID: string
): Promise<TrainServiceDetails> {
  return PebbleTS.fetchJSON(`${config.service}/api/service/${serviceID}`);
}
