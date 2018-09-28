
function [ hd] = notch200Hz(Fs)
% create 2nd order notch filter

% This is commented out so you don't need the filter design toolbox
%[b1,a1] = iirnotch(200/(Fs/2),0.002)
%[b2,a2] = iirnotch(200/(Fs/2),0.002);
%[b3,a3] = iirnotch(200/(Fs/2),0.002);
switch Fs
    case 48000
    %[b1,a1] = iirnotch(200/(Fs/2),0.002)
    b1 = [0.99686823577081 ; -1.99305326784750 ; 0.99686823577081];
    a1 = [1.00000000000000 ; -1.99305326784750 ; 0.99373647154161];
    case 44100
    %[b1,a1] = iirnotch(200/(Fs/2),0.002)
    b1 = [0.99686823577081 ; -1.99292709442701 ; 0.99686823577081];
    a1 = [1.00000000000000 ; -1.99292709442701 ; 0.99373647154161];
    case 32000
    %[b1,a1] = iirnotch(200/(Fs/2),0.002)
    b1 = [0.99686823577081 ; -1.99219937297651 ; 0.99686823577081];
    a1 = [1.00000000000000 ; -1.99219937297651 ; 0.99373647154161];
    otherwise
        fprintf('Error: Unknown Sampling Rate. Should never get here!\n');
end
% Use same coefficients for 2nd and third stage
b2 = b1;
b3 = b1;
a2 = a1;
a3 = a1;
hd = dfilt.df2sos(b1,a1,b2,a2,b3,a3);
%[b,a] = sos2tf(sos,g);
% establish gains at 185Hz (-30dB), 171.5 (-10dB), 158 (-3dB), 138 (-1dB)
f = [185 171.5 158 138];
%h = 20 * log10(abs(freqz(b,a,f,Fs)))
[h,w] = freqz(hd,24000,48000);
semilogx(w,20 * log10(abs(h)));
axis([100 300 -140 0]);
impz(hd);
fprintf('%f %f\n',w(186),20 * log10(abs(h(186))));
fprintf('%f %f\n',w(171),20 * log10(abs(h(171))));
fprintf('%f %f\n',w(158),20 * log10(abs(h(158))));
fprintf('%f %f\n',w(138),20 * log10(abs(h(138))));
