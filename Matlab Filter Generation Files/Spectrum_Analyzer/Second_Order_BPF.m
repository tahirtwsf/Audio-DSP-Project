% Design a 2Nth order bandpass filter
Fs = 48000;
N = 3;
[b7, a7] = butter(N, [1414 / (Fs/2), 2828 / (Fs/2)]);

W = pi * logspace(log10(20),log10(Fs/2),10001)/(Fs/2);
H = freqz(b7, a7, W);
semilogx(W / pi * (Fs/2), 20*log10(abs(H)));
axis([20, Fs/2, -20, 1]);    % Set axis limits for x-axis to [20, 24000] Hz and y-axis to [-20, 1] dB
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');