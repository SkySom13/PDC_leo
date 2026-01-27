# ECU 배포 빠른 시작 가이드

## 🚀 빠른 실행 (개발 PC에서)

### ECU1 (192.168.1.100) - VehicleControl

```bash
# 파일 전송
rsync -avz --exclude='build*' --exclude='.git' \
    app/VehicleControlECU/ \
    app/config/start_ecu1.sh \
    commonapi/ install_folder/ \
    pi@192.168.1.100:~/DES_Head-Unit/

# ECU1에 SSH 접속
ssh pi@192.168.1.100

# 실행
cd ~/DES_Head-Unit/app/config
./start_ecu1.sh
```

### ECU2 (192.168.1.101) - Head Unit

```bash
# 파일 전송
rsync -avz --exclude='build*' --exclude='.git' \
    app/GearApp/ app/AmbientApp/ app/IC_app/ app/MediaApp/ \
    app/config/ \
    commonapi/ install_folder/ \
    pi@192.168.1.101:~/DES_Head-Unit/

# ECU2에 SSH 접속
ssh pi@192.168.1.101

# 실행 (전체 시스템)
cd ~/DES_Head-Unit/app/config
./start_all_ecu2.sh
```

## 📁 새로 생성된 파일들

```
app/config/
├── routing_manager_ecu2.json         # ECU2 라우팅 매니저 설정
├── start_routing_manager_ecu2.sh     # 라우팅 매니저 단독 실행
├── start_all_ecu2.sh                 # ECU2 전체 시스템 실행
└── start_ecu1.sh                     # ECU1 실행
```

## 🔧 수정된 파일들

1. **GearApp/config/vsomeip_ecu2.json**
   - ❌ 제거: `"routing": "client-sample"`
   - ✅ 변경: `"id": "0x0100"` (was 0xFFFF)
   - ✅ 변경: `"name": "GearApp"` (was client-sample)

2. **MediaApp/vsomeip.json**
   - ❌ 제거: `"routing": "MediaApp"`
   - ✅ 변경: `"unicast": "192.168.1.101"` (was 127.0.0.1)
   - ✅ 변경: `"id": "0x1236"` (was 0x1234)
   - ✅ 변경: `"multicast": "224.244.224.245"` (was 224.0.0.1)

3. **AmbientApp/vsomeip_ambient.json** - 이미 OK ✅
4. **IC_app/vsomeip_ic.json** - 이미 OK ✅

## 🎯 핵심 변경사항

### 방법 1 (이전) → 방법 2 (현재)

**이전 (GearApp이 라우팅 매니저):**
```
GearApp [Host] → AmbientApp, IC_app 모두 GearApp에 의존
```

**현재 (전용 라우팅 매니저):**
```
Routing Manager Daemon [Host]
    ↓
GearApp, AmbientApp, IC_app, MediaApp 모두 동등하게 연결
```

**장점:**
- ✅ 앱 독립성: GearApp 재시작 시에도 다른 앱 영향 없음
- ✅ 안정성: 전용 라우팅 매니저가 항상 실행 중
- ✅ 확장성: 새 앱 추가 시 설정만 하면 됨

## 📊 실행 순서

### ECU2 자동 실행 순서 (start_all_ecu2.sh)

1. **클린업**: 기존 프로세스 종료
2. **네트워크 확인**: IP, 멀티캐스트, ECU1 연결
3. **Routing Manager**: 라우팅 데몬 시작
4. **GearApp**: GUI 앱 시작
5. **AmbientApp**: 앰비언트 라이트 앱 시작
6. **IC_app**: 계기판 앱 시작

## 🔍 로그 확인

```bash
# ECU2에서
tail -f /tmp/routing_manager.log  # 라우팅 매니저
tail -f /tmp/gearapp.log           # GearApp
tail -f /tmp/ambientapp.log        # AmbientApp
tail -f /tmp/ic_app.log            # IC_app

# 전체 로그 한번에 보기
tail -f /tmp/*.log
```

## 🛑 전체 종료

```bash
# ECU2
killall -9 GearApp AmbientApp IC_app MediaApp routingmanagerd
sudo rm -rf /tmp/vsomeip-*

# ECU1
killall -9 VehicleControlECU
sudo rm -rf /tmp/vsomeip-*
```

## ⚠️ 트러블슈팅 빠른 체크

```bash
# 1. 라우팅 매니저 실행 확인
ls -la /tmp/vsomeip-0

# 2. 네트워크 확인
ping -c 1 192.168.1.100  # ECU2 → ECU1
ping -c 1 192.168.1.101  # ECU1 → ECU2

# 3. 멀티캐스트 라우트 확인
ip route | grep 224.0.0.0

# 4. 서비스 발견 패킷 확인
sudo tcpdump -i eth0 -n 'udp and port 30490'
```

## 📚 상세 문서

- **전체 배포 가이드**: `/docs/ECU2_DEPLOYMENT_ROUTING_MANAGER.md`
- **트러블슈팅**: `/docs/ECU_COMMUNICATION_TROUBLESHOOTING_GUIDE.md`
- **통신 테스트**: `/docs/전체통신테스트.md`
