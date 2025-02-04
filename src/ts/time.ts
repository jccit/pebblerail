import { TrainTime } from "./types/time";

export interface RenderedTime {
  scheduled: string;
  actual: string;
  lateness: number;
  isLate: boolean;
}

function dateToTimeString(date: Date): string {
  const hours =
    date.getHours().toString().length === 1
      ? "0" + date.getHours()
      : date.getHours().toString();
  const minutes =
    date.getMinutes().toString().length === 1
      ? "0" + date.getMinutes()
      : date.getMinutes().toString();
  return `${hours}:${minutes}`;
}

/**
 * Renders a train time in a human readable format.
 * @param time - Train time object to render
 */
export function calculateTime(time: TrainTime): RenderedTime | null {
  const { scheduled, estimated, actual } = time;
  const target = actual || estimated;

  if (!scheduled && !estimated && !actual) {
    return null;
  }

  // If there is no scheduled time but there is an actual time, show the actual time
  if (!scheduled && target) {
    const targetDate = new Date(target);
    return {
      scheduled: dateToTimeString(targetDate),
      actual: dateToTimeString(targetDate),
      lateness: 0,
      isLate: false,
    };
  }

  // If there is no actual time but there is a scheduled time, show the scheduled time
  if (!target && scheduled) {
    const scheduledDate = new Date(scheduled);
    return {
      scheduled: dateToTimeString(scheduledDate),
      actual: dateToTimeString(scheduledDate),
      lateness: 0,
      isLate: false,
    };
  }

  // If we reach this point and either of them are null, give up
  if (!scheduled || !target) {
    return null;
  }

  // At this point we should have both times available
  // Let's check if the train is late
  const scheduledDate = new Date(scheduled);
  const targetDate = new Date(target);

  return {
    scheduled: dateToTimeString(scheduledDate),
    actual: dateToTimeString(targetDate),
    lateness: Math.floor(
      (targetDate.getTime() - scheduledDate.getTime()) / 60000
    ),
    isLate:
      Math.floor((targetDate.getTime() - scheduledDate.getTime()) / 60000) >= 1,
  };
}
