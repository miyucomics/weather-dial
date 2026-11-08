#include "render.h"

#include "M5Unified.hpp"

LGFX_Sprite canvas(&M5.Display);

constexpr int CX = 120;
constexpr int CY = 120;

constexpr int SUN_RADIUS = 100;
constexpr int TICK_RADIUS = 110;
constexpr int TICK_MAJOR_RADIUS = 100;
constexpr int POINTER_TIP_RADIUS = 110;
constexpr int POINTER_BASE_RADIUS = 100;

constexpr int ROW_DATE = 70;
constexpr int ROW_TIME = 100;
constexpr int ROW_RAIN = 150;
constexpr int ROW_TEMP = 180;

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

constexpr uint16_t COLOR_BACKGROUND = rgb(17, 17, 26);
constexpr uint16_t COLOR_UI = rgb(205, 214, 244);
constexpr uint16_t COLOR_SUN = rgb(249, 226, 175);
constexpr uint16_t COLOR_TICKS = rgb(49, 50, 68);
constexpr uint16_t COLOR_RAIN = rgb(137, 180, 250);

uint16_t rain_color(int pct) {
    auto mix = [&](int dry, int wet) { return dry + (wet - dry) * pct / 100; };
    return rgb(mix(49, 137), mix(50, 180), mix(68, 250));
}

constexpr float PI_F = 3.14159265f;

struct Point {
    float x, y;
};

float hour_angle(float hour) {
    return (hour - 12.0f) * 15.0f * PI_F / 180.0f;
}

float minute_angle(int minutes_since_midnight) {
    return hour_angle(minutes_since_midnight / 60.0f);
}

Point polar(float angle, float radius) {
    return {CX + radius * sinf(angle), CY - radius * cosf(angle)};
}

int px(float v) {
    return (int)lroundf(v);
}

void thick_line(Point a, Point b, float width, uint16_t color) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f)
        return;
    float nx = -dy / len * width / 2;
    float ny = dx / len * width / 2;
    canvas.fillTriangle(px(a.x + nx), px(a.y + ny), px(a.x - nx), px(a.y - ny), px(b.x + nx), px(b.y + ny), color);
    canvas.fillTriangle(px(a.x - nx), px(a.y - ny), px(b.x - nx), px(b.y - ny), px(b.x + nx), px(b.y + ny), color);
}

void draw_sun(Point c, bool rising) {
    int x = px(c.x);
    int y = px(c.y);
    canvas.fillCircle(x, y, 6, COLOR_SUN);
    for (int i = 0; i < 8; i++) {
        float a = i * PI_F / 4;
        canvas.drawLine(px(x + 9 * cosf(a)), px(y + 9 * sinf(a)), px(x + 12 * cosf(a)),
                        px(y + 12 * sinf(a)), COLOR_SUN);
    }

    int dir = rising ? -1 : 1;
    canvas.fillTriangle(x, y + dir * 21, x - 4, y + dir * 14, x + 4, y + dir * 14, COLOR_SUN);
}

void draw_cloud(int x, int y) {
    canvas.fillCircle(x - 6, y, 5, COLOR_UI);
    canvas.fillCircle(x + 1, y - 4, 6, COLOR_UI);
    canvas.fillCircle(x + 8, y, 4, COLOR_UI);
    canvas.fillRect(x - 6, y, 15, 5, COLOR_UI);
    canvas.drawLine(x - 4, y + 8, x - 6, y + 12, COLOR_RAIN);
    canvas.drawLine(x + 2, y + 8, x, y + 12, COLOR_RAIN);
    canvas.drawLine(x + 8, y + 8, x + 6, y + 12, COLOR_RAIN);
}

void draw_ring_and_ticks(const uint8_t *rain) {
    for (int hour = 0; hour < 24; hour++) {
        bool major = hour % 6 == 0;
        float angle = hour_angle((float)hour);
        Point a = polar(angle, CX);
        Point b = polar(angle, major ? TICK_MAJOR_RADIUS : TICK_RADIUS);
        thick_line(a, b, major ? 4.0f : 2.0f, rain_color(rain[hour]));
    }
}

void draw_pointer(const WeatherData &s) {
    float angle = hour_angle(s.hour + s.minute / 60.0f);
    Point tip = polar(angle, POINTER_TIP_RADIUS);
    Point left = polar(angle - 0.03f, POINTER_BASE_RADIUS);
    Point right = polar(angle + 0.03f, POINTER_BASE_RADIUS);
    canvas.fillTriangle(px(tip.x), px(tip.y), px(left.x), px(left.y), px(right.x), px(right.y), COLOR_UI);

    if (s.now < 0)
        return;
    Point position = polar(minute_angle(s.now), POINTER_BASE_RADIUS);
    canvas.fillCircle(px(position.x), px(position.y), 3, COLOR_UI);
}

void draw_readout(const WeatherData &s) {
    const char *MONTHS[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    char buf[32];
    canvas.setTextColor(COLOR_UI);
    canvas.setTextDatum(middle_center);
    canvas.setFont(&fonts::DejaVu12);
    snprintf(buf, sizeof(buf), "%d %s", s.day, MONTHS[s.month - 1]);
    canvas.drawString(buf, CX, ROW_DATE);

    canvas.setFont(&fonts::DejaVu24);
    int hour12 = (s.hour % 12 == 0) ? 12 : s.hour % 12;
    snprintf(buf, sizeof(buf), "%02d:%02d %s", hour12, s.minute, s.hour < 12 ? "AM" : "PM");
    canvas.drawString(buf, CX, ROW_TIME);

    // rain
    canvas.setFont(&fonts::DejaVu18);
    canvas.setTextColor(COLOR_RAIN);
    canvas.setTextDatum(middle_left);

    char mm[16];
    char pct[8];
    snprintf(mm, sizeof(mm), "%.1f mm", s.precip_mm);
    snprintf(pct, sizeof(pct), "%d%%", s.precip_percent);

    const int ICON_W = 26;
    const int GAP = 10;
    int w_mm = canvas.textWidth(mm);
    int w_pct = canvas.textWidth(pct);
    int x = CX - (ICON_W + GAP + w_mm + GAP + w_pct) / 2;

    draw_cloud(x + ICON_W / 2, ROW_RAIN - 3);
    canvas.drawString(mm, x + ICON_W + GAP, ROW_RAIN);
    canvas.drawString(pct, x + ICON_W + GAP + w_mm + GAP, ROW_RAIN);

    // temperature
    canvas.setFont(&fonts::DejaVu12);
    canvas.setTextColor(COLOR_UI);
    canvas.setTextDatum(middle_left);

    char num[8];
    snprintf(num, sizeof(num), "%.0f", s.temperature);
    int number_width = canvas.textWidth(num);
    int unit_width = canvas.textWidth("C");
    int temperature_x = CX - (number_width + 8 + unit_width) / 2;
    canvas.drawString(num, temperature_x, ROW_TEMP);
    canvas.drawCircle(temperature_x + number_width + 3, ROW_TEMP - 6, 2, COLOR_UI);
    canvas.drawString("C", temperature_x + number_width + 8, ROW_TEMP);
}

void draw_status(const char *text) {
    canvas.setFont(&fonts::DejaVu18);
    canvas.setTextColor(COLOR_UI);
    canvas.setTextDatum(middle_center);
    canvas.drawString(text, CX, CY);
}

void render_init() {
    M5.Display.setBrightness(20);
    canvas.createSprite(240, 240);
}

void render_frame(const ViewState &view) {
    const WeatherData &s = view.sample;
    canvas.fillSprite(COLOR_BACKGROUND);

    if (view.has_sample) {
        draw_ring_and_ticks(s.chances);
        draw_sun(polar(minute_angle(s.sunrise), SUN_RADIUS), true);
        draw_sun(polar(minute_angle(s.sunset), SUN_RADIUS), false);
        draw_pointer(s);
        draw_readout(s);
    } else {
        draw_status(view.status_buffer);
    }

    canvas.pushSprite(0, 0);
}
