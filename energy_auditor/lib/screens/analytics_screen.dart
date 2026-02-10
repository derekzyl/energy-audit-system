import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../providers/energy_providers.dart';

class AnalyticsScreen extends ConsumerWidget {
  const AnalyticsScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final readingsAsync = ref.watch(readingsProvider);

    return Scaffold(
      appBar: AppBar(title: const Text('Analytics')),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: readingsAsync.when(
          data: (readings) {
            if (readings.isEmpty)
              return const Center(child: Text("No data for analytics"));

            // Prepare data for chart (reversed because API returns desc)
            final data = readings.take(20).toList().reversed.toList();

            return Column(
              children: [
                const Text(
                  "Power Consumption (Last 20 readings)",
                  style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 20),
                SizedBox(
                  height: 300,
                  child: LineChart(
                    LineChartData(
                      gridData: FlGridData(show: true),
                      titlesData: FlTitlesData(show: false),
                      borderData: FlBorderData(show: true),
                      lineBarsData: [
                        LineChartBarData(
                          spots: data.asMap().entries.map((e) {
                            return FlSpot(
                              e.key.toDouble(),
                              e.value.sensor1Watts,
                            );
                          }).toList(),
                          isCurved: true,
                          color: Colors.blue,
                          dotData: FlDotData(show: false),
                        ),
                        LineChartBarData(
                          spots: data.asMap().entries.map((e) {
                            return FlSpot(
                              e.key.toDouble(),
                              e.value.sensor2Watts,
                            );
                          }).toList(),
                          isCurved: true,
                          color: Colors.red,
                          dotData: FlDotData(show: false),
                        ),
                      ],
                    ),
                  ),
                ),
                const SizedBox(height: 10),
                const Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Icon(Icons.circle, color: Colors.blue, size: 12),
                    SizedBox(width: 5),
                    Text("Sensor 1"),
                    SizedBox(width: 20),
                    Icon(Icons.circle, color: Colors.red, size: 12),
                    SizedBox(width: 5),
                    Text("Sensor 2"),
                  ],
                ),
              ],
            );
          },
          loading: () => const Center(child: CircularProgressIndicator()),
          error: (e, s) => Center(child: Text("Error: $e")),
        ),
      ),
    );
  }
}
