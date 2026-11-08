from dataclasses import dataclass
from datetime import date, datetime, timedelta

URL = (
    "https://api.open-meteo.com/v1/forecast"
    "?latitude=42.729014&longitude=-73.676728"
    "&daily=sunrise,sunset"
    "&hourly=temperature_2m,precipitation_probability,precipitation"
    "&timezone=America%2FNew_York&past_days=3&forecast_days=4"
)


@dataclass
class HourlyState:
    time: datetime
    temperature: float
    precip_chance: int
    precip_mm: float
    sunrise: datetime
    sunset: datetime

    @property
    def is_daylight(self):
        mid = self.time + timedelta(minutes=30)
        return self.sunrise <= mid < self.sunset


def parse_forecast(raw):
    daily = raw["daily"]
    sun_times = {
        date.fromisoformat(d): (
            datetime.fromisoformat(r),
            datetime.fromisoformat(s),
        )
        for d, r, s in zip(daily["time"], daily["sunrise"], daily["sunset"])
    }

    hourly = raw["hourly"]
    readings = []
    for t, temp, precip_chance, precip_mm in zip(
        hourly["time"],
        hourly["temperature_2m"],
        hourly["precipitation_probability"],
        hourly["precipitation"],
    ):
        time = datetime.fromisoformat(t)
        sunrise, sunset = sun_times[time.date()]

        if precip_chance is None:
            precip_chance = 100 if (precip_mm or 0) > 0 else 0

        readings.append(
            HourlyState(
                time=time,
                temperature=temp,
                precip_chance=int(precip_chance),
                precip_mm=precip_mm or 0.0,
                sunrise=sunrise,
                sunset=sunset,
            )
        )

    return readings


class Forecast:
    def __init__(self, readings):
        self._by_time = {r.time: r for r in readings}

    @classmethod
    def from_dict(cls, raw):
        return cls(parse_forecast(raw))

    def at_offset(self, offset_hours):
        target = datetime.now() + timedelta(hours=offset_hours)
        hour_start = target.replace(minute=0, second=0, microsecond=0)
        return target, self._by_time[hour_start]

    def rain_profile(self, day):
        midnight = datetime.combine(day, datetime.min.time())
        return [
            self._by_time[midnight + timedelta(hours=h)].precip_chance
            for h in range(24)
        ]
