import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart';
import 'package:shared_preferences/shared_preferences.dart';

import '../models/energy_models.dart';

class ApiService {
  final Dio _dio = Dio();
  String? _baseUrl;

  ApiService() {
    _initBaseUrl();
  }

  Future<void> _initBaseUrl() async {
    final prefs = await SharedPreferences.getInstance();
    _baseUrl =
        prefs.getString('api_url') ??
        'https://xenophobic-netta-cybergenii-1584fde7.koyeb.app';
    _dio.options.baseUrl = _baseUrl!;
  }

  Future<void> setBaseUrl(String url) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('api_url', url);
    _baseUrl = url;
    _dio.options.baseUrl = url;
  }

  Future<List<EnergyReading>> getReadings(String deviceId) async {
    await _initBaseUrl();
    try {
      final response = await _dio.get('/energy/readings/$deviceId?limit=20');
      return (response.data as List)
          .map((e) => EnergyReading.fromJson(e))
          .toList();
    } catch (e) {
      debugPrint('Error fetching readings: $e');
      return [];
    }
  }

  Future<List<EnergyAlert>> getAlerts(String deviceId) async {
    await _initBaseUrl();
    try {
      final response = await _dio.get(
        '/energy/audit/$deviceId/alerts?limit=10',
      );
      return (response.data as List)
          .map((e) => EnergyAlert.fromJson(e))
          .toList();
    } catch (e) {
      debugPrint('Error fetching alerts: $e');
      return [];
    }
  }

  Future<void> triggerAudit(String deviceId) async {
    await _initBaseUrl();
    try {
      // Assuming audit triggers on post reading
    } catch (e) {
      debugPrint('Error triggering audit: $e');
    }
  }

  Future<EnergyGoalProgress?> getGoalProgress(String deviceId) async {
    await _initBaseUrl();
    try {
      final response = await _dio.get('/energy/goals/progress/$deviceId');
      return EnergyGoalProgress.fromJson(response.data);
    } catch (e) {
      debugPrint('Error fetching goal progress: $e');
      return null;
    }
  }

  Future<bool> setGoal(
    String deviceId,
    double target,
    DateTime start,
    DateTime end,
  ) async {
    await _initBaseUrl();
    try {
      await _dio.post(
        '/energy/goals',
        data: {
          'device_id': deviceId,
          'target_kwh': target,
          'period_start': start.toIso8601String(),
          'period_end': end.toIso8601String(),
        },
      );
      return true;
    } catch (e) {
      debugPrint('Error setting goal: $e');
      return false;
    }
  }
}
