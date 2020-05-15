#ifndef CHART_VALUE_H
#define CHART_VALUE_H

namespace chart_value {
template <typename Col, typename F = typename Col::name>
class chart_value
{
    Col values;
public:
    chart_value() : values{} {}
    chart_value(const Col& col) = delete;
    Col& operator=(const Col& col) = delete;
    Col& operator[](const int pos) {
        if(pos > values.size()) {
            values.resize(pos);
        }
        return values[pos];
    }
};
}
#endif // CHART_VALUE_H
