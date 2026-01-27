# Add Phase 1 test files to Tests.vcxproj

$vcxproj = Get-Content 'Tests\Tests.vcxproj' -Raw

# Find where to insert (after CFringeSegmentFileIOTest.cpp)
$searchPattern = '    <ClCompile Include="DigitModeTests\\CFringeSegmentFileIOTest.cpp">[\s\S]*?    </ClCompile>'

$newTests = @'
    <ClCompile Include="DigitModeTests\CFringeSegmentFileIOTest.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="DigitModeTests\SelectionManagerTest.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="DigitModeTests\HitTesterTest.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="DigitModeTests\CommandDispatcherTest.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="DigitModeTests\InputHandlerTest.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
'@

$updated = $vcxproj -replace $searchPattern, $newTests

Set-Content 'Tests\Tests.vcxproj' -Value $updated -NoNewline

Write-Host "✓ Updated Tests.vcxproj with Phase 1 test files" -ForegroundColor Green
