import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_riverpod/legacy.dart';

import '../models/energy_models.dart';
import '../services/api_service.dart';

final apiServiceProvider = Provider<ApiService>((ref) => ApiService());

final deviceIdProvider = StateProvider<String>(
  (ref) => "ESP32_ENERGY_MONITOR",
); // Default/Mock

final readingsProvider = FutureProvider.autoDispose<List<EnergyReading>>((
  ref,
) async {
  final api = ref.watch(apiServiceProvider);
  final deviceId = ref.watch(deviceIdProvider);
  return api.getReadings(deviceId);
});

final alertsProvider = FutureProvider.autoDispose<List<EnergyAlert>>((
  ref,
) async {
  final api = ref.watch(apiServiceProvider);
  final deviceId = ref.watch(deviceIdProvider);
  return api.getAlerts(deviceId);
});

final goalProgressProvider = FutureProvider.autoDispose<EnergyGoalProgress?>((
  ref,
) async {
  final api = ref.watch(apiServiceProvider);
  final deviceId = ref.watch(deviceIdProvider);
  return api.getGoalProgress(deviceId);
});
