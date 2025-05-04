# ATI-AnimationToImages

🎞️ 애니메이션 → 이미지 추출기
> 동영상(.mp4, .webm, .gif)을 **지정한 FPS 기준으로 이미지 프레임으로 변환**하는 Windows용 프로그램입니다.

> 단일 변환 및 \*\*일괄 변환(batch mode)\*\*을 모두 지원합니다.

## 💡 사용 방법

### 단일 모드
<div align="center">
  <table>
    <tr>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">1</div>
        <img src="https://github.com/user-attachments/assets/5a1c9af5-a012-4c3a-8d0e-4f19891a8977" alt="Midori_Title" width="300"/>
      </td>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">2</div>
        <img src="https://github.com/user-attachments/assets/ec5587a8-d33f-412c-8bbb-ee35af9a7c3f" alt="Momoi_Title" width="300"/>
      </td>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">3</div>
        <img src="https://github.com/user-attachments/assets/11e4a02b-d68c-4d20-a451-7bd3c53ad1e5" alt="Momoi_Title" width="300"/>
      </td>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">4</div>
        <img src="https://github.com/user-attachments/assets/00f08d94-078b-4371-a24e-9765a46a04ab" alt="Momoi_Title" width="300"/>
      </td>
    </tr>
  </table>
</div>



1. `애니메이션 → 이미지 추출기.exe` 실행
2. FPS 값, 폴더명, 접두사 입력
3. `[변환 시작]` 클릭 → 프레임 추출 후 결과가 표시됩니다
4. 썸네일이 우측 하단에 표시되며, 에러 로그나 결과는 리스트에 출력됩니다


### 일괄 모드

<div align="center">
  <table>
    <tr>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">1</div>
        <img src="https://github.com/user-attachments/assets/22bc9788-3b6d-45ed-bd81-d569e9ca8321" alt="Midori_Title" width="300"/>
      </td>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">2</div>
        <img src="https://github.com/user-attachments/assets/cedb8d05-f7e5-4bda-9ff4-3670c706793a" alt="Momoi_Title" width="300"/>
      </td>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">3</div>
        <img src="https://github.com/user-attachments/assets/8355bbe8-3095-487b-b013-336475b4547c" alt="Momoi_Title" width="300"/>
      </td>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">4</div>
        <img src="https://github.com/user-attachments/assets/2fc9db9a-9002-42cc-abc2-024d31083e26" alt="Momoi_Title" width="300"/>
      </td>
      <td align="center" valign="top" style="padding: 10px;">
        <div style="margin-bottom: 8px; font-weight: bold; font-size: 18px;">5</div>
        <img src="https://github.com/user-attachments/assets/2998c457-70a3-416b-8060-6ae2e98ea7d6" alt="Momoi_Title" width="300"/>
      </td>
    </tr>
  </table>
</div>


1. 실행 후 `[일괄 변환 창 열기]` 클릭
2. `[폴더 선택]` 버튼 클릭 → 변환할 비디오가 포함된 상위 폴더 선택
3. 자동으로 하위 모든 비디오 파일을 탐색하여 리스트업
4. 각각의 **출력 폴더명**과 **파일 접두사** 수정 가능
5. 전체에 대해 공통 FPS 설정 가능
6. `[일괄 변환 실행]` 클릭 → 진행 결과는 아래 리스트에 출력됨

---

## 📌 주요 기능

* 🎬 **동영상 → 이미지 변환**
  원하는 FPS 값을 기준으로 동영상에서 프레임 이미지를 추출하여 `.png` 파일로 저장합니다.

* 📁 **저장 폴더 및 파일명 접두사 지정**
  프레임 이미지가 저장될 폴더 이름과 파일명 접두사를 커스터마이징할 수 있습니다.
  예: `frame_0001.png`, `frame_0002.png`, ...

* 📷 **썸네일 미리보기 지원 (단일 모드)**
  선택한 동영상에서 1프레임을 추출하여 UI에 표시합니다.

* 🧾 **일괄 변환(Batch Mode)**
  선택한 폴더 내의 모든 `.mp4`, `.webm`, `.gif` 파일을 하위 폴더까지 자동 탐색 후,
  각각 개별 설정으로 한꺼번에 프레임 추출을 실행합니다.

---

## 🔍 출력 예시

* FPS: `60`
* 영상 길이: `10초`
* 추출된 이미지 수: **600장**
  → 파일명 예시: `frame_0001.png`, `frame_0002.png`, ..., `frame_0600.png`

---

## 🛠 시스템 요구사항

* Windows 10 이상
* FFmpeg 파일 필요 

---

## 🧰 FFmpeg 명령어 형식

```bash
ffmpeg -i [입력파일] -vf fps=[FPS] "[출력폴더]/[접두사]_%04d.png"
```

---

## 📝 개발 특징

* Win32 API + GDI+ 기반 GUI
* Gdiplus를 활용한 썸네일 미리보기 구현
* 비동기 변환 실행 (std::thread)
* 파일 탐색기 연동 (`ShellExecuteW` 사용)
```
case ID_BUTTON_START_CONVERT:
    if (wcslen(g_SelectedFile) == 0) {
        ShowError(L"먼저 비디오 파일을 선택하세요.");
        return 0;
    }
    std::thread(RunFFmpegConvert).detach(); // 🔸 비동기 실행
    break;
```
* `std::filesystem`으로 하위 디렉토리 순회 및 클린업

---

## 📦 빌드 또는 실행 시 주의사항

* `ffmpeg` 프로그램이 필요합니다.
* 프로그램은 별도의 설치 없이 실행 가능하며, 관리자 권한 불필요합니다

---