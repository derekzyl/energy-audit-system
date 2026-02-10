import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:google_fonts/google_fonts.dart';

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
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        title: Text(
          'Energy Auditor',
          style: GoogleFonts.outfit(fontWeight: FontWeight.bold),
        ),
        backgroundColor: Theme.of(
          context,
        ).scaffoldBackgroundColor.withOpacity(0.8),
        elevation: 0,
        actions: [
          IconButton(
            icon: const Icon(Icons.analytics_rounded),
            onPressed: () => Navigator.push(
              context,
              MaterialPageRoute(builder: (_) => const AnalyticsScreen()),
            ),
          ),
          IconButton(
            icon: const Icon(Icons.settings_rounded),
            onPressed: () => Navigator.push(
              context,
              MaterialPageRoute(builder: (_) => const DeviceSetupScreen()),
            ),
          ),
        ],
      ),
      body: RefreshIndicator(
        onRefresh: () async {
          ref.invalidate(readingsProvider);
          ref.invalidate(alertsProvider);
          ref.invalidate(goalProgressProvider);
        },
        child: SingleChildScrollView(
          physics: const AlwaysScrollableScrollPhysics(),
          padding: const EdgeInsets.fromLTRB(16, 100, 16, 16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              _buildReadingsSection(readingsAsync),
              const SizedBox(height: 24),
              Text(
                "Monthly Goal",
                style: GoogleFonts.outfit(
                  fontSize: 20,
                  fontWeight: FontWeight.bold,
                  color: Colors.white,
                ),
              ),
              const SizedBox(height: 12),
              _buildGoalSection(ref),
              const SizedBox(height: 24),
              Text(
                "System Alerts",
                style: GoogleFonts.outfit(
                  fontSize: 20,
                  fontWeight: FontWeight.bold,
                  color: Colors.white,
                ),
              ),
              const SizedBox(height: 12),
              _buildAlertsSection(alertsAsync, context),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildReadingsSection(AsyncValue<List<EnergyReading>> readingsAsync) {
    return readingsAsync.when(
      data: (readings) {
        if (readings.isEmpty) {
          return const Center(
            child: Padding(
              padding: EdgeInsets.all(32.0),
              child: Text('Waiting for data... Check connections.'),
            ),
          );
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
                    latest.sensor1Voltage,
                    Colors.blueAccent,
                    Colors.lightBlueAccent,
                  ),
                ),
                const SizedBox(width: 16),
                Expanded(
                  child: _buildSensorCard(
                    "Sensor 2",
                    latest.sensor2Watts,
                    latest.sensor2Amps,
                    latest.sensor2Voltage,
                    Colors.orangeAccent,
                    Colors.deepOrangeAccent,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 16),
            _buildEnvironmentCard(latest),
          ],
        );
      },
      loading: () => const Center(child: CircularProgressIndicator()),
      error: (e, stack) => Center(child: Text('Error: $e')),
    );
  }

  Widget _buildSensorCard(
    String title,
    double watts,
    double amps,
    double voltage,
    Color colorStart,
    Color colorEnd,
  ) {
    return Container(
      decoration: BoxDecoration(
        gradient: LinearGradient(
          colors: [colorStart.withOpacity(0.2), colorEnd.withOpacity(0.1)],
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
        ),
        borderRadius: BorderRadius.circular(20),
        border: Border.all(color: colorStart.withOpacity(0.3)),
      ),
      padding: const EdgeInsets.all(16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                title,
                style: GoogleFonts.outfit(
                  fontWeight: FontWeight.w600,
                  color: Colors.white70,
                ),
              ),
              Icon(Icons.flash_on_rounded, color: colorStart, size: 18),
            ],
          ),
          const SizedBox(height: 12),
          Text(
            "${watts.toStringAsFixed(0)} W",
            style: GoogleFonts.outfit(
              fontSize: 28,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
          const SizedBox(height: 4),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                "${amps.toStringAsFixed(2)} A",
                style: GoogleFonts.outfit(color: Colors.white54, fontSize: 13),
              ),
              // Voltage Indicator
              Container(
                padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
                decoration: BoxDecoration(
                  color: (voltage < 200 || voltage > 250)
                      ? Colors.red.withOpacity(0.3)
                      : Colors.green.withOpacity(0.3),
                  borderRadius: BorderRadius.circular(4),
                ),
                child: Text(
                  "${voltage.toStringAsFixed(0)}V",
                  style: GoogleFonts.outfit(
                    fontSize: 12,
                    fontWeight: FontWeight.bold,
                    color: (voltage < 200 || voltage > 250)
                        ? Colors.redAccent
                        : Colors.greenAccent,
                  ),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildEnvironmentCard(EnergyReading reading) {
    return Container(
      decoration: BoxDecoration(
        color: const Color(0xFF1E1E24),
        borderRadius: BorderRadius.circular(20),
        border: Border.all(color: Colors.white10),
      ),
      padding: const EdgeInsets.symmetric(vertical: 20, horizontal: 16),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceAround,
        children: [
          _buildEnvItem(Icons.thermostat, "${reading.temperature}°C", "Temp"),
          _buildEnvItem(Icons.water_drop, "${reading.humidity}%", "Humidity"),
          _buildEnvItem(Icons.light_mode, "${reading.lightLux} lx", "Light"),
        ],
      ),
    );
  }

  Widget _buildEnvItem(IconData icon, String value, String label) {
    return Column(
      children: [
        Icon(icon, color: Colors.white54, size: 24),
        const SizedBox(height: 8),
        Text(
          value,
          style: GoogleFonts.outfit(
            fontSize: 16,
            fontWeight: FontWeight.bold,
            color: Colors.white,
          ),
        ),
        Text(
          label,
          style: GoogleFonts.outfit(fontSize: 12, color: Colors.white38),
        ),
      ],
    );
  }

  Widget _buildAlertsSection(
    AsyncValue<List<EnergyAlert>> alertsAsync,
    BuildContext context,
  ) {
    return alertsAsync.when(
      data: (alerts) {
        if (alerts.isEmpty) {
          return Container(
            width: double.infinity,
            padding: const EdgeInsets.all(24),
            decoration: BoxDecoration(
              color: Colors.green.withOpacity(0.1),
              borderRadius: BorderRadius.circular(16),
              border: Border.all(color: Colors.green.withOpacity(0.3)),
            ),
            child: Column(
              children: [
                const Icon(
                  Icons.check_circle_outline_rounded,
                  color: Colors.greenAccent,
                  size: 48,
                ),
                const SizedBox(height: 12),
                Text(
                  "All Systems Optimal",
                  style: GoogleFonts.outfit(
                    color: Colors.greenAccent,
                    fontWeight: FontWeight.bold,
                    fontSize: 16,
                  ),
                ),
              ],
            ),
          );
        }
        return Column(
          children: alerts.map((alert) {
            Color color = Colors.blue;
            if (alert.severity == 'warning') color = Colors.amber;
            if (alert.severity == 'critical' || alert.severity == 'danger')
              color = Colors.red;

            return Container(
              margin: const EdgeInsets.only(bottom: 12),
              decoration: BoxDecoration(
                color: color.withOpacity(0.1),
                border: Border(left: BorderSide(color: color, width: 4)),
                borderRadius: const BorderRadius.only(
                  topRight: Radius.circular(12),
                  bottomRight: Radius.circular(12),
                ),
              ),
              child: ListTile(
                contentPadding: const EdgeInsets.symmetric(
                  horizontal: 16,
                  vertical: 8,
                ),
                leading: Icon(Icons.warning_amber_rounded, color: color),
                title: Text(
                  alert.auditType.replaceAll('_', ' ').toUpperCase(),
                  style: GoogleFonts.outfit(
                    fontWeight: FontWeight.bold,
                    fontSize: 14,
                    color: color,
                  ),
                ),
                subtitle: Text(
                  alert.message,
                  style: GoogleFonts.outfit(color: Colors.white70),
                ),
                trailing: alert.estimatedWasteWatts > 0
                    ? Container(
                        padding: const EdgeInsets.symmetric(
                          horizontal: 8,
                          vertical: 4,
                        ),
                        decoration: BoxDecoration(
                          color: Colors.red.withOpacity(0.2),
                          borderRadius: BorderRadius.circular(8),
                        ),
                        child: Text(
                          "${alert.estimatedWasteWatts.round()}W",
                          style: const TextStyle(
                            color: Colors.redAccent,
                            fontSize: 12,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      )
                    : null,
              ),
            );
          }).toList(),
        );
      },
      loading: () => const LinearProgressIndicator(),
      error: (e, s) => Text("Error loading alerts: $e"),
    );
  }

  Widget _buildGoalSection(WidgetRef ref) {
    final goalAsync = ref.watch(goalProgressProvider);
    return goalAsync.when(
      data: (goal) {
        if (goal == null || !goal.hasGoal) {
          return GestureDetector(
            onTap: () => _showSetGoalDialog(ref.context, ref),
            child: Container(
              width: double.infinity,
              padding: const EdgeInsets.all(24),
              decoration: BoxDecoration(
                color: Colors.white.withOpacity(0.05),
                borderRadius: BorderRadius.circular(16),
                border: Border.all(
                  color: Colors.white10,
                  style: BorderStyle.solid,
                ), // Dashed border simulated
              ),
              child: Column(
                children: [
                  const Icon(
                    Icons.flag_rounded,
                    color: Colors.white54,
                    size: 32,
                  ),
                  const SizedBox(height: 8),
                  Text(
                    "Set Monthly Goal",
                    style: GoogleFonts.outfit(color: Colors.white70),
                  ),
                ],
              ),
            ),
          );
        }

        final percent = goal.percentage / 100.0;
        final isOver = percent > 1.0;

        return Container(
          padding: const EdgeInsets.all(20),
          decoration: BoxDecoration(
            gradient: LinearGradient(
              colors: [const Color(0xFF2E3250), const Color(0xFF1E1E24)],
              begin: Alignment.topLeft,
              end: Alignment.bottomRight,
            ),
            borderRadius: BorderRadius.circular(20),
            boxShadow: [
              BoxShadow(
                color: Colors.black.withOpacity(0.2),
                blurRadius: 10,
                offset: const Offset(0, 4),
              ),
            ],
          ),
          child: Column(
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        "Monthly Usage",
                        style: GoogleFonts.outfit(
                          color: Colors.white54,
                          fontSize: 12,
                        ),
                      ),
                      const SizedBox(height: 4),
                      Text(
                        "${goal.consumedKwh.toStringAsFixed(2)} kWh",
                        style: GoogleFonts.outfit(
                          fontSize: 24,
                          fontWeight: FontWeight.bold,
                          color: Colors.white,
                        ),
                      ),
                    ],
                  ),
                  Column(
                    crossAxisAlignment: CrossAxisAlignment.end,
                    children: [
                      Text(
                        "Target",
                        style: GoogleFonts.outfit(
                          color: Colors.white54,
                          fontSize: 12,
                        ),
                      ),
                      const SizedBox(height: 4),
                      Text(
                        "${goal.targetKwh} kWh",
                        style: GoogleFonts.outfit(
                          fontSize: 16,
                          fontWeight: FontWeight.w600,
                          color: Colors.white70,
                        ),
                      ),
                    ],
                  ),
                ],
              ),
              const SizedBox(height: 16),
              ClipRRect(
                borderRadius: BorderRadius.circular(8),
                child: LinearProgressIndicator(
                  value: percent.clamp(0.0, 1.0),
                  backgroundColor: Colors.white10,
                  color: isOver ? Colors.redAccent : const Color(0xFF00E676),
                  minHeight: 12,
                ),
              ),
              const SizedBox(height: 8),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text(
                    "${goal.percentage}% Used",
                    style: GoogleFonts.outfit(
                      color: isOver
                          ? Colors.redAccent
                          : const Color(0xFF00E676),
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  TextButton(
                    onPressed: () => _showSetGoalDialog(ref.context, ref),
                    style: TextButton.styleFrom(
                      padding: EdgeInsets.zero,
                      minimumSize: Size.zero,
                      tapTargetSize: MaterialTapTargetSize.shrinkWrap,
                    ),
                    child: Text(
                      "Edit",
                      style: GoogleFonts.outfit(
                        color: Colors.white54,
                        fontSize: 12,
                      ),
                    ),
                  ),
                ],
              ),
            ],
          ),
        );
      },
      loading: () => const LinearProgressIndicator(),
      error: (e, s) => const SizedBox(),
    );
  }

  void _showSetGoalDialog(BuildContext context, WidgetRef ref) {
    final controller = TextEditingController();
    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        backgroundColor: const Color(0xFF1E1E24),
        title: Text(
          "Set Monthly Goal",
          style: GoogleFonts.outfit(color: Colors.white),
        ),
        content: TextField(
          controller: controller,
          keyboardType: TextInputType.number,
          style: const TextStyle(color: Colors.white),
          decoration: InputDecoration(
            labelText: "Target kWh",
            labelStyle: const TextStyle(color: Colors.white54),
            enabledBorder: OutlineInputBorder(
              borderSide: BorderSide(color: Colors.white10),
            ),
            focusedBorder: OutlineInputBorder(
              borderSide: BorderSide(color: Colors.blue),
            ),
          ),
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
