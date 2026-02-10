import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../models/energy_models.dart';
import '../providers/energy_providers.dart';
import 'analytics_screen.dart';
import 'device_setup_screen.dart';

class DashboardScreen extends ConsumerWidget {
  const DashboardScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final readingsAsync = ref.watch(readingsProvider);
    final alertsAsync = ref.watch(alertsProvider);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Energy Dashboard'),
        actions: [
          IconButton(
            icon: const Icon(Icons.analytics),
            onPressed: () => Navigator.push(
              context,
              MaterialPageRoute(builder: (_) => const AnalyticsScreen()),
            ),
          ),
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: () => Navigator.push(
              context,
              MaterialPageRoute(builder: (_) => const DeviceSetupScreen()),
            ),
          ),
        ],
      ),
      body: RefreshIndicator(
        onRefresh: () async {
          // Force refresh of providers
          ref.invalidate(readingsProvider);
          ref.invalidate(alertsProvider);
          ref.invalidate(goalProgressProvider);
        },
        child: ListView(
          padding: const EdgeInsets.all(16.0),
          children: [
            _buildReadingsSection(readingsAsync),
            _buildReadingsSection(readingsAsync),
            const SizedBox(height: 16),
            const Text(
              "Energy Goal",
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            _buildGoalSection(ref),
            const SizedBox(height: 16),
            const Text(
              "Active Alerts",
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            _buildAlertsSection(alertsAsync),
          ],
        ),
      ),
    );
  }

  Widget _buildReadingsSection(AsyncValue<List<EnergyReading>> readingsAsync) {
    return readingsAsync.when(
      data: (readings) {
        if (readings.isEmpty) {
          return const Center(child: Text('No data waiting...'));
        }
        final latest = readings.first;
        return Column(
          children: [
            Row(
              children: [
                Expanded(
                  child: _buildSensorCard(
                    "Sensor 1",
                    latest.sensor1Watts,
                    latest.sensor1Amps,
                  ),
                ),
                const SizedBox(width: 10),
                Expanded(
                  child: _buildSensorCard(
                    "Sensor 2",
                    latest.sensor2Watts,
                    latest.sensor2Amps,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 10),
            Card(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.spaceAround,
                  children: [
                    Column(
                      children: [
                        const Icon(Icons.thermostat),
                        Text("${latest.temperature}°C"),
                      ],
                    ),
                    Column(
                      children: [
                        const Icon(Icons.water_drop),
                        Text("${latest.humidity}%"),
                      ],
                    ),
                    Column(
                      children: [
                        const Icon(Icons.light_mode),
                        Text("${latest.lightLux} lx"),
                      ],
                    ),
                  ],
                ),
              ),
            ),
          ],
        );
      },
      loading: () => const Center(child: CircularProgressIndicator()),
      error: (e, stack) => Center(child: Text('Error: $e')),
    );
  }

  Widget _buildSensorCard(String title, double watts, double amps) {
    return Card(
      color: watts > 1000 ? Colors.red.shade100 : Colors.blue.shade50,
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            Text(title, style: const TextStyle(fontWeight: FontWeight.bold)),
            const SizedBox(height: 10),
            Text(
              "${watts.toStringAsFixed(1)} W",
              style: const TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
            ),
            Text("${amps.toStringAsFixed(2)} A"),
          ],
        ),
      ),
    );
  }

  Widget _buildAlertsSection(AsyncValue<List<EnergyAlert>> alertsAsync) {
    return alertsAsync.when(
      data: (alerts) {
        if (alerts.isEmpty) {
          return const Padding(
            padding: EdgeInsets.all(20),
            child: Text("No active alerts. System efficient."),
          );
        }
        return Column(
          children: alerts
              .map(
                (alert) => Card(
                  child: ListTile(
                    leading: Icon(
                      Icons.warning,
                      color: _getSeverityColor(alert.severity),
                    ),
                    title: Text(alert.auditType),
                    subtitle: Text(alert.message),
                    trailing: Text(
                      "${alert.estimatedWasteWatts.toStringAsFixed(0)}W waste",
                    ),
                  ),
                ),
              )
              .toList(),
        );
      },
      loading: () => const LinearProgressIndicator(),
      error: (e, s) => Text("Error loading alerts"),
    );
  }

  Color _getSeverityColor(String severity) {
    if (severity == 'critical') return Colors.red;
    if (severity == 'warning') return Colors.orange;
    return Colors.blue;
  }

  Widget _buildGoalSection(WidgetRef ref) {
    final goalAsync = ref.watch(goalProgressProvider);
    return goalAsync.when(
      data: (goal) {
        if (goal == null || !goal.hasGoal) {
          return Card(
            child: ListTile(
              title: const Text("No Monthly Goal Set"),
              trailing: ElevatedButton(
                onPressed: () => _showSetGoalDialog(ref.context, ref),
                child: const Text("Set Goal"),
              ),
            ),
          );
        }
        return Card(
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    Text("Target: ${goal.targetKwh} kWh"),
                    Text("${goal.percentage}% Used"),
                  ],
                ),
                const SizedBox(height: 10),
                LinearProgressIndicator(
                  value: (goal.percentage / 100).clamp(0, 1).toDouble(),
                  backgroundColor: Colors.grey.shade300,
                  color: goal.percentage > 90 ? Colors.red : Colors.green,
                  minHeight: 10,
                ),
                const SizedBox(height: 10),
                Text(
                  "${goal.consumedKwh.toStringAsFixed(2)} kWh consumed this month",
                ),
                Align(
                  alignment: Alignment.centerRight,
                  child: TextButton(
                    onPressed: () => _showSetGoalDialog(ref.context, ref),
                    child: const Text("Edit Goal"),
                  ),
                ),
              ],
            ),
          ),
        );
      },
      loading: () => const LinearProgressIndicator(),
      error: (e, s) => const Text("Error loading goal"),
    );
  }

  void _showSetGoalDialog(BuildContext context, WidgetRef ref) {
    final controller = TextEditingController();
    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        title: const Text("Set Monthly Goal (kWh)"),
        content: TextField(
          controller: controller,
          keyboardType: TextInputType.number,
          decoration: const InputDecoration(labelText: "Target kWh"),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: const Text("Cancel"),
          ),
          ElevatedButton(
            onPressed: () async {
              final target = double.tryParse(controller.text);
              if (target != null) {
                final deviceId = ref.read(deviceIdProvider);
                final now = DateTime.now();
                final start = DateTime(now.year, now.month, 1);
                final end = DateTime(now.year, now.month + 1, 0);

                final success = await ref
                    .read(apiServiceProvider)
                    .setGoal(deviceId, target, start, end);
                if (success) {
                  ref.invalidate(goalProgressProvider);
                  if (context.mounted) Navigator.pop(ctx);
                }
              }
            },
            child: const Text("Save"),
          ),
        ],
      ),
    );
  }
}
