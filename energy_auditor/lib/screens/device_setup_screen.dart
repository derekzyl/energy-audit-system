import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shared_preferences/shared_preferences.dart';

import '../models/energy_models.dart';
import '../providers/energy_providers.dart';
import 'dashboard_screen.dart';

class DeviceSetupScreen extends ConsumerStatefulWidget {
  const DeviceSetupScreen({super.key});

  @override
  ConsumerState<DeviceSetupScreen> createState() => _DeviceSetupScreenState();
}

class _DeviceSetupScreenState extends ConsumerState<DeviceSetupScreen> {
  final _urlController = TextEditingController();
  final _deviceIdController = TextEditingController();

  @override
  void initState() {
    super.initState();
    _loadSettings();
  }

  Future<void> _loadSettings() async {
    final prefs = await SharedPreferences.getInstance();
    _urlController.text =
        prefs.getString('api_url') ??
        'https://xenophobic-netta-cybergenii-1584fde7.koyeb.app';
    _deviceIdController.text =
        prefs.getString('device_id') ?? 'ESP32_ENERGY_MONITOR';
  }

  Future<void> _saveSettings() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('api_url', _urlController.text);
    await prefs.setString('device_id', _deviceIdController.text);

    // Update providers
    ref.read(apiServiceProvider).setBaseUrl(_urlController.text);
    ref.read(deviceIdProvider.notifier).state = _deviceIdController.text;

    if (mounted) {
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(const SnackBar(content: Text('Settings Saved')));
      Navigator.pushReplacement(
        context,
        MaterialPageRoute(builder: (_) => const DashboardScreen()),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Device Setup')),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            TextField(
              controller: _urlController,
              decoration: const InputDecoration(
                labelText: 'Backend URL',
                hintText: 'http://192.168.X.X:8000',
              ),
            ),
            const SizedBox(height: 16),
            TextField(
              controller: _deviceIdController,
              decoration: const InputDecoration(
                labelText: 'Device ID',
                hintText: 'ESP32_...',
              ),
            ),
            const SizedBox(height: 32),
            ElevatedButton(
              onPressed: _saveSettings,
              child: const Text('Save & Connect'),
            ),
            const SizedBox(height: 32),
            const Divider(color: Colors.white24),
            const SizedBox(height: 16),
            const Text(
              "Sensor Configuration",
              style: TextStyle(
                fontSize: 18,
                fontWeight: FontWeight.bold,
                color: Colors.white,
              ),
            ),
            const SizedBox(height: 16),
            _buildSensorConfig(1),
            const SizedBox(height: 16),
            _buildSensorConfig(2),
          ],
        ),
      ),
    );
  }

  Widget _buildSensorConfig(int sensorNum) {
    final configsAsync = ref.watch(sensorConfigProvider);

    return configsAsync.when(
      data: (configs) {
        final config = configs.firstWhere(
          (c) => c.sensorNumber == sensorNum,
          orElse: () => EnergySensorConfig(
            sensorNumber: sensorNum,
            customLabel: "Sensor $sensorNum",
            applianceCategory: "General",
          ),
        );

        return Card(
          color: Colors.white.withOpacity(0.05),
          child: Padding(
            padding: const EdgeInsets.all(12.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  "Sensor $sensorNum",
                  style: const TextStyle(
                    fontWeight: FontWeight.bold,
                    color: Colors.white70,
                  ),
                ),
                const SizedBox(height: 8),
                Row(
                  children: [
                    Expanded(
                      flex: 2,
                      child: DropdownButtonFormField<String>(
                        initialValue:
                            [
                              "General",
                              "Lighting",
                              "AC",
                              "Heater",
                              "Fridge",
                              "Other",
                            ].contains(config.applianceCategory)
                            ? config.applianceCategory
                            : "General",
                        dropdownColor: const Color(0xFF2E3250),
                        style: const TextStyle(color: Colors.white),
                        decoration: const InputDecoration(
                          labelText: "Category",
                          labelStyle: TextStyle(color: Colors.white54),
                          enabledBorder: OutlineInputBorder(
                            borderSide: BorderSide(color: Colors.white24),
                          ),
                        ),
                        items:
                            [
                              "General",
                              "Lighting",
                              "AC",
                              "Heater",
                              "Fridge",
                              "Other",
                            ].map((String value) {
                              return DropdownMenuItem<String>(
                                value: value,
                                child: Text(value),
                              );
                            }).toList(),
                        onChanged: (val) {
                          if (val != null) {
                            _updateConfig(sensorNum, config.customLabel, val);
                          }
                        },
                      ),
                    ),
                    const SizedBox(width: 8),
                    Expanded(
                      flex: 3,
                      child: TextFormField(
                        initialValue: config.customLabel,
                        style: const TextStyle(color: Colors.white),
                        decoration: const InputDecoration(
                          labelText: "Label",
                          labelStyle: TextStyle(color: Colors.white54),
                          enabledBorder: OutlineInputBorder(
                            borderSide: BorderSide(color: Colors.white24),
                          ),
                        ),
                        onFieldSubmitted: (val) {
                          _updateConfig(
                            sensorNum,
                            val,
                            config.applianceCategory,
                          );
                        },
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        );
      },
      loading: () => const LinearProgressIndicator(),
      error: (e, s) =>
          Text("Error: $e", style: const TextStyle(color: Colors.red)),
    );
  }

  Future<void> _updateConfig(
    int sensorNum,
    String label,
    String category,
  ) async {
    final deviceId = ref.read(deviceIdProvider);
    final success = await ref
        .read(apiServiceProvider)
        .updateSensorConfig(
          deviceId,
          EnergySensorConfig(
            sensorNumber: sensorNum,
            customLabel: label,
            applianceCategory: category,
          ),
        );
    if (success) {
      ref.invalidate(sensorConfigProvider);
      if (mounted) {
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(const SnackBar(content: Text('Sensor Config Saved')));
      }
    }
  }
}
